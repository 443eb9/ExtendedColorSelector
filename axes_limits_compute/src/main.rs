use std::{
    error::Error,
    fs::File,
    io::{Read, Write, stdin},
    path::PathBuf,
};

use bevy_color::{ColorToComponents, Laba, Lcha, LinearRgba, Oklaba, Oklcha, Xyza};
use rayon::iter::{
    IndexedParallelIterator, IntoParallelIterator, IntoParallelRefMutIterator, ParallelIterator,
};

const SEGMENTS: u32 = 256;

type TransferFunc = fn([f32; 3]) -> [f32; 3];

fn min_opt<T: Ord>(lhs: Option<T>, rhs: T) -> Option<T> {
    match lhs {
        Some(lhs) => Some(std::cmp::min(lhs, rhs)),
        None => Some(rhs),
    }
}

fn max_opt<T: Ord>(lhs: Option<T>, rhs: T) -> Option<T> {
    match lhs {
        Some(lhs) => Some(std::cmp::max(lhs, rhs)),
        None => Some(rhs),
    }
}

fn compute_axes_limits(transfer: TransferFunc, segments: u32) -> Vec<f32> {
    let mut result = vec![[0.0f32, 0.0, 0.0, 0.0]; 3 * segments as usize];
    result.par_iter_mut().enumerate().for_each(|(i, data)| {
        let mut min_x = None;
        let mut max_x = None;
        let mut min_y = None;
        let mut max_y = None;

        let locked_index = i as u32 / segments;
        let locked = i as u32 % segments;
        let max_value = segments as f32 - 1.0;

        for x in 0..segments {
            for y in 0..segments {
                let locked = locked as f32;
                let fx = x as f32;
                let fy = y as f32;

                let color = {
                    match locked_index {
                        0 => [locked / max_value, fx / max_value, fy / max_value],
                        1 => [fx / max_value, locked / max_value, fy / max_value],
                        2 => [fx / max_value, fy / max_value, locked / max_value],
                        _ => unreachable!(),
                    }
                };

                let transferred = transfer(color);
                if transferred.into_iter().any(|x| x < 0.0 || x > 1.0) {
                    continue;
                }

                min_x = min_opt(min_x, x);
                max_x = max_opt(max_x, x);
                min_y = min_opt(min_y, y);
                max_y = max_opt(max_y, y);
            }
        }

        let min_x = min_x.unwrap_or(0);
        let max_x = max_x.unwrap_or(min_x);
        let min_y = min_y.unwrap_or(0);
        let max_y = max_y.unwrap_or(min_y);

        data[0] = min_x as f32 / max_value;
        data[1] = min_y as f32 / max_value;
        data[2] = max_x as f32 / max_value;
        data[3] = max_y as f32 / max_value;
    });

    result.into_flattened()
}

fn toe_inv(x: f32) -> f32 {
    const K_1: f32 = 0.206;
    const K_2: f32 = 0.03;
    const K_3: f32 = (1.0 + K_1) / (1.0 + K_2);
    (x * x + K_1 * x) / (K_3 * (x + K_2))
}

fn map_oklxx(color: [f32; 3]) -> [f32; 3] {
    // [toe_inv(color[0]), color[1], color[2]]
    color
}

macro_rules! define_transfer_function {
    ($model: ty, $transfer_fn_name: ident, $min_max_values: expr, $mapper: ident) => {
        fn $transfer_fn_name(mut color: [f32; 3]) -> [f32; 3] {
            color.iter_mut().enumerate().for_each(|(i, c)| {
                let mn = $min_max_values[i][0];
                let mx = $min_max_values[i][1];
                *c = mn * (1.0 - *c) + mx * *c
            });
            color = $mapper(color);
            LinearRgba::from(<$model>::new(color[0], color[1], color[2], 1.0))
                .to_f32_array_no_alpha()
        }
    };

    ($model: ty, $transfer_fn_name: ident, $min_max_values: expr) => {
        fn $transfer_fn_name(mut color: [f32; 3]) -> [f32; 3] {
            color.iter_mut().enumerate().for_each(|(i, c)| {
                let mn = $min_max_values[i][0];
                let mx = $min_max_values[i][1];
                *c = mn * (1.0 - *c) + mx * *c
            });
            LinearRgba::from(<$model>::new(color[0], color[1], color[2], 1.0))
                .to_f32_array_no_alpha()
        }
    };
}

define_transfer_function!(
    Oklaba,
    oklab_transfer,
    [[0.0, 1.0], [-1.0, 1.0], [-1.0, 1.0]],
    map_oklxx
);
define_transfer_function!(Xyza, xyz_transfer, [[0.0, 1.0], [0.0, 1.0], [0.0, 1.0]]);
define_transfer_function!(Laba, lab_transfer, [[0.0, 1.5], [-1.5, 1.5], [-1.5, 1.5]]);
define_transfer_function!(Lcha, lch_transfer, [[0.0, 1.5], [0.0, 1.5], [0.0, 360.0]]);
define_transfer_function!(
    Oklcha,
    oklch_transfer,
    [[0.0, 1.0], [0.0, 1.0], [0.0, 360.0]],
    map_oklxx
);

fn find_plugin_dir() -> Result<PathBuf, Box<dyn Error>> {
    let mut current = std::env::current_dir()?;
    while !current.join("extended_color_selector.desktop").exists() {
        current = current
            .parent()
            .ok_or("Search reached root folder, but still can't find the folder containing .desktop file")?
            .to_path_buf();
    }

    let dev_env = current.join("src");
    if dev_env.exists() {
        return Ok(dev_env);
    }

    Err("Unable to find the plugin directory".into())
}

fn main() {
    let plugin_dir = find_plugin_dir().unwrap();
    println!("Found plugin directory: {:?}", &plugin_dir);
    let mut file = File::create(plugin_dir.join("axes_limits.bytes")).unwrap();

    for (transfer, name) in [
        (xyz_transfer as TransferFunc, "XYZ"),
        (lab_transfer, "LAB"),
        (lch_transfer, "LCH"),
        (oklab_transfer, "OkLab"),
        (oklch_transfer, "OkLch"),
    ] {
        println!("Computing for {} color model.", name);
        let limits = compute_axes_limits(transfer, SEGMENTS + 1)
            .into_par_iter()
            .flat_map(|x| x.to_ne_bytes())
            .collect::<Vec<_>>();
        file.write(&limits).unwrap();
    }

    println!("Done! Now you can close this program!");

    // Pause the program.
    stdin().read(&mut [0]).unwrap();
}

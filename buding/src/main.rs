mod t_;
use t_::*;
mod num2fmtstr_;
mod other_;

fn z__() {
    println!("{:?}", if_buzu__(2, 0, 0));
    /*{
        let s = "";
        println!("{}=len={}", s, buding::gjke4_sp_len__(s));
    }*/
    if let Err(s) = z2__() {
        println!("err: {}", s);
    }
}
fn z2__() -> Result<(), String> {
    let mut ret:Vec<String> = vec![];
    other_::stoc__(&["1a啊".to_string(), "码".to_string()], &mut ret)?;
    println!("{:?}", ret);
    Ok(())
}

fn main() {
     z__();
}


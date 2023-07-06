
pub fn stoi__(arg:&[String], i:usize) -> Result<i64, String> {
	if arg.len() > i {
		match arg[i].parse::<i64>() {
			Ok(i) => Ok(i),
			Err(e) => Err(e.to_string())
		}
	} else {Ok(0)}
}

pub fn if_buzu__(len:usize, l1:usize, l2:usize) -> Result<(), String> {
	if len < l1 {
		Err("参数不足".to_string())
	} else if len > l2 {
		Err("参数超量".to_string())
	} else {
		Ok(())
	}
}

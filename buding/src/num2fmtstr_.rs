use super::t_::*;

#[no_mangle]
extern fn fmtstr2num__(arg:&[String], ret:&mut Vec<String>) -> Result<(), String> {
	if_buzu__(arg.len(), 2, usize::MAX)?;
	let mut ret2:f64 = 0.0;
	let mut jinzhi = vec![];
	for i in 1..arg.len() {
		match arg[i].parse::<f64>() {
			Ok(f) => {
				jinzhi.push(f)
			}
			Err(e) => return Err(e.to_string())
		}
	}
	let src = arg[0].chars().rev();
	let mut buf = String::new();
	let mut wei = 0;
	let mut suan = |buf:&mut String| {
		if !buf.is_empty() {
			let mut shu4 = match buf.parse::<f64>() {
				Ok(f) => f,
				Err(e) => return Err(e.to_string())
			};
			let mut jinzhi_i:i32 = wei - 1;
			while jinzhi_i >= 0 {
				let mut jinzh_i=jinzhi.len() as i32-1-jinzhi_i;
				if jinzh_i<0 {
					jinzh_i=0;
				}
				shu4 *= jinzhi[jinzh_i as usize];
				jinzhi_i -= 1;
			}
			ret2 += shu4;
			wei += 1;
			buf.clear();
		}
		Ok(())
	};
	for c in src {
		if c >= '0' && c <= '9' || c == '.' {
			buf.insert(0, c);
			continue;
		}
		suan(&mut buf)?;
	}
	suan(&mut buf)?;
	ret.push(ret2.to_string());
	Ok(())
}

#[no_mangle]
extern fn num2fmtstr__(arg:&[String], ret:&mut Vec<String>) -> Result<(), String> {
	if_buzu__(arg.len(), 2, usize::MAX)?;
	let mut ret2 = String::new();
	let mut jinzhi = vec![];
	let mut fenge = vec![];
	for i in 1..arg.len() {
		let s = &arg[i];
		if i % 2 == 1 {
			match s.parse::<f64>() {
				Ok(f) => {
					jinzhi.insert(0, f)
				}
				Err(e) => return Err(e.to_string())
			}
		} else {
			fenge.insert(0, s)
		}
	}
	let mut src = match arg[0].parse::<f64>() {
		Ok(f) => f,
		Err(e) => return Err(e.to_string())
	};
	let mut jinzh_i=0;
	loop {
		let quan=jinzhi[if jinzh_i<jinzhi.len() {jinzh_i} else {jinzhi.len() - 1}];
		let shu=src%quan;
		/*
		rust formatter Remove following 0
		https://doc.rust-lang.org/std/fmt/#syntax
		:.0}", shu
		*/
		ret2.insert_str(0, &format!("{}", shu as u32));
		src/=quan;
		if src < 1.0 {
			break
		}
		ret2.insert_str(0, fenge[if jinzh_i < fenge.len() {jinzh_i} else {fenge.len() - 1}]);
		jinzh_i += 1;
	}
	ret.push(ret2.to_string());
	Ok(())
}

use zhscript2::{u_::*};

pub fn if_buzu__(len:usize, l1:usize, l2:usize) -> Result2_ {
	if len < l1 {
		result2_::err2__("参数不足")
	} else if len > l2 {
		result2_::err2__("参数超量")
	} else {
		ok__()
	}
}

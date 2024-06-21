pub fn by_path__<K>(path:&String, i:usize, max:usize, sp:&str, fskip:&K) -> String
where
K:Fn(&str) -> bool
{
	let ss:Vec<&str> = path[i..].split("/").collect();
	let end = ss.len() - 1;
	let mut s = String::new();
	let mut idx = ss.len();
	let mut skip = false;
	'l1: loop {
		if idx == 0 {break}
		idx -= 1;
		let i = ss[idx];
		if fskip(i) {
			skip = true;
			continue 'l1;
		}
		let mut start = 0;
		for idx2 in 0..idx {
			let i2 = ss[idx2];
			if i.starts_with(i2) && start < i2.len() {
				start = i2.len()
			}
		}
		let mut end2 = i.len();
		if start >= end2 {continue}
		if !skip && !s.is_empty() {s.insert_str(0, sp)}
		if idx == end {
			if let Some(idx) = i.rfind('.') {
				end2 = idx;
				if start > end2 {
					end2 = start;
				}
			}
		}
		skip = false;
		s.insert_str(0, &i[start..end2]);
		if s.len() >= max {
			if start > 0 {s.insert_str(0, &i[0..start]);}
			break
		}
	}
	s
}

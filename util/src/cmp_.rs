use std::cmp::Ordering;
pub fn bb__(s1:&[u8], s2:&[u8]) -> Ordering {
    let mut i1 = 0;
    let mut i2 = 0;
    let mut n1 = String::new();
    let mut n2 = String::new();
    loop {
        let b1 = i1 >= s1.len();
        let b2 = i2 >= s2.len();
        if b1 {
            return if b2 {Ordering::Equal} else {Ordering::Less}
        }
        if b2 {
            return Ordering::Greater
        }
        let c1 = s1[i1];
        let c2 = s2[i2];
        //println!("{:?} {:?} {:?} {:?} {:?} {:?}", c1 as char,c2 as char,i1,i2, c1,c2);
        let to_n1 = c1 >= b'0' && c1 <= b'9';
        let to_n2 = c2 >= b'0' && c2 <= b'9';
        if to_n1 {
            n1.push(c1 as char);
            i1 += 1;
        }
        if to_n2 {
            n2.push(c2 as char);
            i2 += 1;
        }
        if to_n1 || to_n2 {
            continue
        }
        if !n1.is_empty() || !n2.is_empty() {
            if n1.is_empty() {
                return if c1 < b'0' {Ordering::Less} else {Ordering::Greater}
            }
            if n2.is_empty() {
                return if c2 > b'0' {Ordering::Less} else {Ordering::Greater}
            }
            let u1:u64 = n1.parse().unwrap();
            let u2:u64 = n2.parse().unwrap();
            //println!("{:20} <>\n{:20}", u1,u2);
            if u1 < u2 {
                return Ordering::Less
            }
            if u1 > u2 {
                return Ordering::Greater
            }
            n1.clear();
            n2.clear();
        }
        if c1 == b'.' && c2 != b'.' {
            return Ordering::Less
        }
        if c2 == b'.' && c1 != b'.' {
            return Ordering::Greater
        }
        if c1 < c2 {
            return Ordering::Less
        }
        if c1 > c2 {
            return Ordering::Greater
        }
        i1 += 1;
        i2 += 1;
    }
}

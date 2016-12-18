fn main() {
    let mut v = Vec::new();
    v.push(1);
    v.push(2);
    v.push(3);

    {
        let ref mut v4 = v;
        // let mut foo: () = v4;

        v4[0] = 2;

        // println!("v2[0] is: {}", v2[0]);
        // println!("v3[0] is: {}", v3[0]);
        println!("v4[0] is: {}", v4[0]);
    }

    let v2 = &v;
    let v3 = &v;

    println!("v[0] is: {}", v[0]);
}

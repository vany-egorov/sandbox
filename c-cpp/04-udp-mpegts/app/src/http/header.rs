use std::collections::HashMap;
use std::collections::hash_map::Iter;


#[derive(Default)]
pub struct Header {
    m: HashMap<String, Vec<String>>,
}

impl<'a> IntoIterator for &'a Header {
    type Item = (&'a String, &'a Vec<String>);
    type IntoIter = Iter<'a, String, Vec<String>>;

    fn into_iter(self) -> Self::IntoIter {
        self.m.iter()
    }
}

impl Header {
    pub fn new() -> Header {
        Header {
            m: HashMap::new(),
        }
    }

    // Add adds the key, value pair to the header.
    // It appends to any existing values associated with key.
    pub fn add(&mut self, k: String, v: String) {
        self.m
            .entry(k)
            .or_insert(Vec::new())
            .push(v);
    }

    // Set sets the header entries associated with key to the single element value.
    // It replaces any existing values associated with key.
    pub fn set(&mut self, k: String, v: String) {
        let vec = self.m
            .entry(k)
            .or_insert(Vec::new());
        vec.clear();
        vec.push(v);
    }

    pub fn get_first(&mut self, k: String) -> Option<&String> {
        match self.m.get(&k) {
            Some(vec) => vec.first(),
            None => None,
        }
    }
}

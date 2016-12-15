use std::collections::HashMap;


#[derive(Default)]
pub struct Header {
    m: HashMap<String, Vec<String>>,
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
        if !self.m.contains_key(&k) {
            let mut vec = Vec::new();
            vec.push(v);
            self.m.insert(k, vec);
        } else {
            let mut vec = self.m.get_mut(&k).unwrap();
            vec.push(v);
        }
    }

    // Set sets the header entries associated with key to the single element value.
    // It replaces any existing values associated with key.
    pub fn set(&mut self, k: String, v: String) {
        if !self.m.contains_key(&k) {
            let mut vec = Vec::new();
            vec.push(v);
            self.m.insert(k, vec);
        } else {
            let mut vec = self.m.get_mut(&k).unwrap();
            vec.clear();
            vec.push(v);
        }
    }

    pub fn get_first(&mut self, k: String) -> Option<&String> {
        match self.m.get(&k) {
            Some(vec) => vec.first(),
            None => None,
        }
    }

    pub fn data(&self) -> &HashMap<String, Vec<String>> { return &self.m }
}

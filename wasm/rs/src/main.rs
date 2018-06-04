extern crate stdweb;


use stdweb::web::{
    document,
    IParentNode,  // query_selector
    INode,  // append_child
};


fn main() {
    stdweb::initialize();

    let body = document()
        .query_selector("body")
        .unwrap()
        .unwrap();

    let div1 = document().create_element("div").unwrap();
    let h1 = document().create_element("h1").unwrap();

    let div11 = document().create_element("div").unwrap();
    let div111 = document().create_element("div").unwrap();
    let div112 = document().create_element("div").unwrap();
    let div113 = document().create_element("div").unwrap();
    let input_src = document().create_element("input").unwrap();
    let input_dst = document().create_element("input").unwrap();
    let button = document().create_element("button").unwrap();
    h1.set_text_content("rendered using rust!");
    button.set_text_content("exchange");

    div1.append_child(&h1);
    div1.append_child(&div11);
    div11.append_child(&div111);
    div111.append_child(&input_src);
    div11.append_child(&div112);
    div112.append_child(&input_dst);
    div11.append_child(&div113);
    div113.append_child(&button);
    body.append_child(&div1);

    stdweb::event_loop();
}

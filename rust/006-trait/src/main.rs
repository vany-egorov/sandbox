trait Factorizer {
    type Actor: Actor;

    fn start(&mut self) -> Self::Actor;
    fn before_finish(&mut self) {
        println!("default -> before_finish");
    }
    fn finish(&mut self, _: Self::Actor) {
    }
}

impl<F, A> Factorizer for F
    where A: Actor, F: FnMut() -> A
{
    type Actor = A;

    fn start(&mut self) -> A {
        self()
    }
}

trait Actor {
    fn start(&mut self) { println!("default -> start"); }
    fn perform(&mut self) { println!("default -> perform"); }
    fn finish(&mut self) { println!("default -> finish"); }
}

#[derive(Debug, Eq, PartialEq)]
struct Action {}

impl Action {
    fn new() -> Action { Action{} }
}

impl Actor for Action {
    fn start(&mut self) { println!("action -> start"); }
    fn perform(&mut self) { println!("action -> perform"); }
    fn finish(&mut self) { println!("action -> finish"); }
}

impl<F> Actor for F
    where F: FnMut()
{
    fn perform(&mut self) {
        self();
    }
}


fn execute<F>(mut f: F)
    where
        F: Factorizer,
{
    let mut a = f.start();

    a.start();
    a.perform();
    a.finish();

    f.before_finish();
    f.finish(a);
}


fn main() {
    execute(|| {
        println!("factory -> start");
        Action::new()
    });

    println!("");

    execute(|| {
        println!("factory/clojure -> start");

        || {
            println!("action/clojure -> start");
        }
    });
}

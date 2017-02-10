extern crate libc;

use self::libc::{sockaddr_in};
use self::libc::{size_t, sem_t, pthread_mutex_t};
use self::libc::{c_int, c_uint, c_void};


#[repr(C)]
pub struct FIFO {
    pub data: *mut u8,
    pub len: size_t,
    pub cap: size_t,
    pub start: size_t,
    pub finish: size_t,
    pub sem: sem_t,
    pub rw_mutex: pthread_mutex_t,
}

#[repr(C)]
pub struct UDP {
    pub sock: c_int,
    pub addrlen: c_uint,
    pub addr: sockaddr_in,
}

#[repr(C)]
pub struct IOReader {
    pub w: *mut c_void,
    pub read: IOReaderReadFunc,
}

pub type IOReaderReadFunc =
    ::std::option::Option<unsafe extern "C" fn(
        ctx: *mut c_void,
        buf: *mut u8,
        bufsz: size_t,
        n: *mut size_t
    ) -> c_int>;

#[repr(C)]
pub struct IOWriter {
    pub w: *mut c_void,
    pub write: IOWriterWriteFunc,
}

#[repr(C)]
pub struct IOMultiWriter {
    pub writers: *mut *mut IOWriter,
    pub len: size_t,
}

pub type IOWriterWriteFunc =
    ::std::option::Option<unsafe extern "C" fn(
        ctx: *mut c_void,
        buf: *mut u8,
        bufsz: size_t,
        n: *mut size_t
    ) -> c_int>;

#![allow(dead_code)]


use std::fmt;


#[derive(Debug)]
pub enum Status {
    Continue           = 100, // RFC 7231, 6.2.1
    SwitchingProtocols = 101, // RFC 7231, 6.2.2
    Processing         = 102, // RFC 2518, 10.1

    OK                   = 200, // RFC 7231, 6.3.1
    Created              = 201, // RFC 7231, 6.3.2
    Accepted             = 202, // RFC 7231, 6.3.3
    NonAuthoritativeInfo = 203, // RFC 7231, 6.3.4
    NoContent            = 204, // RFC 7231, 6.3.5
    ResetContent         = 205, // RFC 7231, 6.3.6
    PartialContent       = 206, // RFC 7233, 4.1
    MultiStatus          = 207, // RFC 4918, 11.1
    AlreadyReported      = 208, // RFC 5842, 7.1
    IMUsed               = 226, // RFC 3229, 10.4.1

    MultipleChoices  = 300, // RFC 7231, 6.4.1
    MovedPermanently = 301, // RFC 7231, 6.4.2
    Found            = 302, // RFC 7231, 6.4.3
    SeeOther         = 303, // RFC 7231, 6.4.4
    NotModified      = 304, // RFC 7232, 4.1
    UseProxy         = 305, // RFC 7231, 6.4.5

    TemporaryRedirect = 307, // RFC 7231, 6.4.7
    PermanentRedirect = 308, // RFC 7538, 3

    BadRequest                   = 400, // RFC 7231, 6.5.1
    Unauthorized                 = 401, // RFC 7235, 3.1
    PaymentRequired              = 402, // RFC 7231, 6.5.2
    Forbidden                    = 403, // RFC 7231, 6.5.3
    NotFound                     = 404, // RFC 7231, 6.5.4
    MethodNotAllowed             = 405, // RFC 7231, 6.5.5
    NotAcceptable                = 406, // RFC 7231, 6.5.6
    ProxyAuthRequired            = 407, // RFC 7235, 3.2
    RequestTimeout               = 408, // RFC 7231, 6.5.7
    Conflict                     = 409, // RFC 7231, 6.5.8
    Gone                         = 410, // RFC 7231, 6.5.9
    LengthRequired               = 411, // RFC 7231, 6.5.10
    PreconditionFailed           = 412, // RFC 7232, 4.2
    RequestEntityTooLarge        = 413, // RFC 7231, 6.5.11
    RequestURITooLong            = 414, // RFC 7231, 6.5.12
    UnsupportedMediaType         = 415, // RFC 7231, 6.5.13
    RequestedRangeNotSatisfiable = 416, // RFC 7233, 4.4
    ExpectationFailed            = 417, // RFC 7231, 6.5.14
    Teapot                       = 418, // RFC 7168, 2.3.3
    UnprocessableEntity          = 422, // RFC 4918, 11.2
    Locked                       = 423, // RFC 4918, 11.3
    FailedDependency             = 424, // RFC 4918, 11.4
    UpgradeRequired              = 426, // RFC 7231, 6.5.15
    PreconditionRequired         = 428, // RFC 6585, 3
    TooManyRequests              = 429, // RFC 6585, 4
    RequestHeaderFieldsTooLarge  = 431, // RFC 6585, 5
    UnavailableForLegalReasons   = 451, // RFC 7725, 3

    InternalServerError           = 500, // RFC 7231, 6.6.1
    NotImplemented                = 501, // RFC 7231, 6.6.2
    BadGateway                    = 502, // RFC 7231, 6.6.3
    ServiceUnavailable            = 503, // RFC 7231, 6.6.4
    GatewayTimeout                = 504, // RFC 7231, 6.6.5
    HTTPVersionNotSupported       = 505, // RFC 7231, 6.6.6
    VariantAlsoNegotiates         = 506, // RFC 2295, 8.1
    InsufficientStorage           = 507, // RFC 4918, 11.5
    LoopDetected                  = 508, // RFC 5842, 7.2
    NotExtended                   = 510, // RFC 2774, 7
    NetworkAuthenticationRequired = 511, // RFC 6585, 6
}

impl fmt::Display for Status {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match *self {
            Status::Continue           => write!(f, "Continue"),
            Status::SwitchingProtocols => write!(f, "Switching Protocols"),
            Status::Processing         => write!(f, "Processing"),

            Status::OK                   => write!(f, "OK"),
            Status::Created              => write!(f, "Created"),
            Status::Accepted             => write!(f, "Accepted"),
            Status::NonAuthoritativeInfo => write!(f, "Non-Authoritative Information"),
            Status::NoContent            => write!(f, "No Content"),
            Status::ResetContent         => write!(f, "Reset Content"),
            Status::PartialContent       => write!(f, "Partial Content"),
            Status::MultiStatus          => write!(f, "Multi-Status"),
            Status::AlreadyReported      => write!(f, "Already Reported"),
            Status::IMUsed               => write!(f, "IM Used"),

            Status::MultipleChoices   => write!(f, "Multiple Choices"),
            Status::MovedPermanently  => write!(f, "Moved Permanently"),
            Status::Found             => write!(f, "Found"),
            Status::SeeOther          => write!(f, "See Other"),
            Status::NotModified       => write!(f, "Not Modified"),
            Status::UseProxy          => write!(f, "Use Proxy"),
            Status::TemporaryRedirect => write!(f, "Temporary Redirect"),
            Status::PermanentRedirect => write!(f, "Permanent Redirect"),

            Status::BadRequest                   => write!(f, "Bad Request"),
            Status::Unauthorized                 => write!(f, "Unauthorized"),
            Status::PaymentRequired              => write!(f, "Payment Required"),
            Status::Forbidden                    => write!(f, "Forbidden"),
            Status::NotFound                     => write!(f, "Not Found"),
            Status::MethodNotAllowed             => write!(f, "Method Not Allowed"),
            Status::NotAcceptable                => write!(f, "Not Acceptable"),
            Status::ProxyAuthRequired            => write!(f, "Proxy Authentication Required"),
            Status::RequestTimeout               => write!(f, "Request Timeout"),
            Status::Conflict                     => write!(f, "Conflict"),
            Status::Gone                         => write!(f, "Gone"),
            Status::LengthRequired               => write!(f, "Length Required"),
            Status::PreconditionFailed           => write!(f, "Precondition Failed"),
            Status::RequestEntityTooLarge        => write!(f, "Request Entity Too Large"),
            Status::RequestURITooLong            => write!(f, "Request URI Too Long"),
            Status::UnsupportedMediaType         => write!(f, "Unsupported Media Type"),
            Status::RequestedRangeNotSatisfiable => write!(f, "Requested Range Not Satisfiable"),
            Status::ExpectationFailed            => write!(f, "Expectation Failed"),
            Status::Teapot                       => write!(f, "I'm a teapot"),
            Status::UnprocessableEntity          => write!(f, "Unprocessable Entity"),
            Status::Locked                       => write!(f, "Locked"),
            Status::FailedDependency             => write!(f, "Failed Dependency"),
            Status::UpgradeRequired              => write!(f, "Upgrade Required"),
            Status::PreconditionRequired         => write!(f, "Precondition Required"),
            Status::TooManyRequests              => write!(f, "Too Many Requests"),
            Status::RequestHeaderFieldsTooLarge  => write!(f, "Request Header Fields Too Large"),
            Status::UnavailableForLegalReasons   => write!(f, "Unavailable For Legal Reasons"),

            Status::InternalServerError           => write!(f, "Internal Server Error"),
            Status::NotImplemented                => write!(f, "Not Implemented"),
            Status::BadGateway                    => write!(f, "Bad Gateway"),
            Status::ServiceUnavailable            => write!(f, "Service Unavailable"),
            Status::GatewayTimeout                => write!(f, "Gateway Timeout"),
            Status::HTTPVersionNotSupported       => write!(f, "HTTP Version Not Supported"),
            Status::VariantAlsoNegotiates         => write!(f, "Variant Also Negotiates"),
            Status::InsufficientStorage           => write!(f, "Insufficient Storage"),
            Status::LoopDetected                  => write!(f, "Loop Detected"),
            Status::NotExtended                   => write!(f, "Not Extended"),
            Status::NetworkAuthenticationRequired => write!(f, "Network Authentication Required"),
        }
    }
}

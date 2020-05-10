package node

import (
	"github.com/hashicorp/memberlist"
)

type Peer struct {
	addr string

	memberlistNode *memberlist.Node
}

func NewPeer(addr string) *Peer {
	p := new(Peer)
	p.addr = addr
	return p
}

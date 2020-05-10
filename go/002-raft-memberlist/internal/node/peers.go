package node

import (
	"net"
	"sync"
)

// TODO: peers mutex
type Peers struct {
	sync.RWMutex

	ps []*Peer
}

func (ps *Peers) Len() int {
	ps.RLock()
	defer ps.RUnlock()

	return len(ps.ps)
}

func (ps *Peers) ForEach(cb func(*Peer) bool) bool {
	ps.RLock()
	defer ps.RUnlock()

	return ps.forEach(cb)
}

func (ps *Peers) forEach(cb func(*Peer) bool) bool {
	for _, p := range ps.ps {
		if !cb(p) {
			return false
		}
	}

	return true
}

func (ps *Peers) push(p *Peer) {
	ps.Lock()
	defer ps.Unlock()

	ps.ps = append(ps.ps, p)
}

func (ps Peers) pluckAddr() (addrs []string) {
	ps.RLock()
	defer ps.RUnlock()

	for _, p := range ps.ps {
		addrs = append(addrs, p.addr)
	}
	return
}

func (ps Peers) getByNameOrIpAddr(name string, addr net.IP) *Peer {
	ps.RLock()
	defer ps.RUnlock()

	for _, p := range ps.ps {
		if p.addr == name || p.addr == addr.String() {
			return p
		}
	}
	return nil
}

func NewPeersFromConfig(peers []configPeer) *Peers {
	ps := new(Peers)

	for _, p := range peers {
		ps.push(NewPeer(p.Addr))
	}

	return ps
}

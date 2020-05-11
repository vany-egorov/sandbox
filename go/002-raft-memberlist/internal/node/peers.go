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

func (ps *Peers) pluckNameWithoutSelf() (addrs []string) {
	ps.RLock()
	defer ps.RUnlock()

	for _, p := range ps.ps {
		if p.self {
			continue
		}
		addrs = append(addrs, p.name)
	}
	return
}

func (ps *Peers) getByNameOrIpAddr(name string, addr net.IP) *Peer {
	ps.RLock()
	defer ps.RUnlock()

	for _, p := range ps.ps {
		if p.name == name || p.name == addr.String() {
			return p
		}
	}
	return nil
}

func (ps *Peers) selfID() uint64 {
	ps.RLock()
	defer ps.RUnlock()

	for _, p := range ps.ps {
		if p.self {
			return p.id
		}
	}

	return 0
}

func NewPeersFromConfig(selfName string, peers []configPeer) *Peers {
	ps := new(Peers)

	selfp := NewPeer(selfName)
	selfp.self = true
	ps.push(selfp)

	for _, p := range peers {
		ps.push(NewPeer(p.Addr))
	}

	return ps
}

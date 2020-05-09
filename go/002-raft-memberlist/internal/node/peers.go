package node

type Peers []*Peer

func (ps Peers) pluckAddr() (addrs []string) {
	for _, p := range ps {
		addrs = append(addrs, p.Addr)
	}
	return
}

package node

import (
	"fmt"
	"regexp"
	"strconv"

	"github.com/hashicorp/memberlist"
)

var peerIDRe = regexp.MustCompile("(?P<id>[0-9]*)$")

type Peer struct {
	id uint64

	// can be either "host" or "host:port" pair
	// or fadn
	name string

	self bool

	memberlistNode *memberlist.Node
}

func (p *Peer) setIDFromName() error {
	variantMatches := peerIDRe.FindAllStringSubmatch(p.name, -1)
	for _, match := range variantMatches {
		for i, name := range peerIDRe.SubexpNames() {
			if i == 0 {
				continue
			}

			if name == "id" {
				ids := match[i]
				if id, err := strconv.ParseUint(ids, 10, 64); err != nil {
					return fmt.Errorf("error parse id from name (%s/%s): %w", name, ids, err)
				} else {
					p.id = id
				}
			}
		}
	}

	return nil
}

func NewPeer(name string) *Peer {
	p := new(Peer)
	p.name = name
	p.setIDFromName()
	return p
}

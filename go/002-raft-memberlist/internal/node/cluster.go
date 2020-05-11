package node

import (
	"context"
	"fmt"
	"strings"
	"sync"
	"time"

	"github.com/cenkalti/backoff/v4"
	"github.com/go-x-pkg/bufpool"
	"github.com/go-x-pkg/log"
	"github.com/hashicorp/memberlist"
)

type clusterLogOutput struct {
	*cluster
}

func (clo *clusterLogOutput) Write(p []byte) (n int, err error) {
	s := strings.TrimSuffix(string(p), "\n")
	clo.fnLog(log.Info, "<<< %s", s)
	return len(s), nil
}

type cluster struct {
	peers *Peers
	fnLog log.FnT

	listCfg    *memberlist.Config
	listEvents chan memberlist.NodeEvent
}

func (c *cluster) notifyJoin(n *memberlist.Node) {
	if p := c.peers.getByNameOrIpAddr(n.Name, n.Addr); p != nil {
		p.memberlistNode = n
	} else {
		p = NewPeer(n.Name)
		p.memberlistNode = n
		c.peers.push(p)
	}
	c.fnLog(log.Info, "[+] %s", n)
}

func (c *cluster) notifyLeave(n *memberlist.Node) {
	if p := c.peers.getByNameOrIpAddr(n.Name, n.Addr); p != nil {
		// TODO: delete node?
		p.memberlistNode = nil
	}
	c.fnLog(log.Warn, "[-] %s", n)
}

func (c *cluster) notifyUpdate(n *memberlist.Node) {
	c.fnLog(log.Info, "[~] %s", n)
}

func (c *cluster) Start(ctx context.Context) error {
	if ctx == nil {
		ctx = context.TODO()
	}

	go func() {
		ticker := time.NewTicker(20 * time.Second)
		defer ticker.Stop()

		buf := bufpool.NewBuf()
		defer buf.Release()

		startRaftOnce := sync.Once{}

		for {
			select {
			case nodeEvent := <-c.listEvents:
				switch nodeEvent.Event {
				case memberlist.NodeJoin:
					c.notifyJoin(nodeEvent.Node)

					if c.peers.Len() == 3 { // TODO: move 3 to config
						startRaftOnce.Do(func() {
							clstrRft := clusterRaft{}
							clstrRft.init(c.fnLog, c.peers)
							go clstrRft.start(ctx)
						})
					}

				case memberlist.NodeLeave:
					c.notifyLeave(nodeEvent.Node)
				case memberlist.NodeUpdate:
					c.notifyUpdate(nodeEvent.Node)
				}
			case <-ctx.Done():
				return

			case <-ticker.C: // log down memberlist state
				buf.Reset()

				// TODO: move to func and more pretty log
				c.peers.ForEach(func(p *Peer) bool {
					fmt.Fprint(buf, p.name)
					buf.WriteByte('(')
					fmt.Fprintf(buf, "0x%02X", p.id)
					if n := p.memberlistNode; n != nil {
						fmt.Fprintf(buf, ", %s", n.Addr)
					}
					if p.self {
						fmt.Fprint(buf, ", self")
					}
					buf.WriteByte(')')
					buf.WriteString("; ")
					return true
				})

				c.fnLog(log.Info, "%s", buf.String())
			}
		}
	}()

	list, err := memberlist.Create(c.listCfg)
	if err != nil {
		return fmt.Errorf("memberlist creation failed: %w", err)
	}

	bckff := backoff.WithContext(backoff.NewExponentialBackOff(), ctx)
	ticker := backoff.NewTicker(bckff)

	for range ticker.C {
		if _, err := list.Join(c.peers.pluckNameWithoutSelf()); err != nil {
			c.fnLog(log.Error, "failed to create cluster: %s. will retry.", err)
			continue
		}

		ticker.Stop()
		break
	}

	return nil
}

func (c *cluster) init(fnLog log.FnT, peers *Peers) error {
	if fnLog == nil {
		fnLog = log.LogfStd
	}
	c.fnLog = fnLog

	c.peers = peers

	c.listEvents = make(chan memberlist.NodeEvent)

	cfg := memberlist.DefaultLANConfig()
	cfg.Events = &memberlist.ChannelEventDelegate{
		Ch: c.listEvents,
	}
	// cfg.Name = "$(fqdn)"
	// cfg.BindPort = 7946
	cfg.LogOutput = &clusterLogOutput{c}
	cfg.TCPTimeout = 10 * time.Second
	cfg.PushPullInterval = 30 * time.Second
	cfg.ProbeTimeout = 500 * time.Millisecond
	cfg.ProbeInterval = time.Second
	// cfg.GossipNodes = 3
	cfg.GossipInterval = 200 * time.Millisecond
	cfg.GossipToTheDeadTime = 30 * time.Second

	c.listCfg = cfg

	return nil
}

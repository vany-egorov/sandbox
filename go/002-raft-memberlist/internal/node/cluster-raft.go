package node

import (
	"context"
	"fmt"
	"time"

	"github.com/go-x-pkg/log"
	"go.etcd.io/etcd/raft"
)

type clusterRaft struct {
	peers *Peers
	fnLog log.FnT
}

func (cr *clusterRaft) start(ctx context.Context) {
	if ctx == nil {
		ctx = context.TODO()
	}

	storage := raft.NewMemoryStorage()

	cfg := &raft.Config{
		ID:              cr.peers.selfID(),
		ElectionTick:    10,
		HeartbeatTick:   1,
		Storage:         storage,
		MaxSizePerMsg:   4096,
		MaxInflightMsgs: 256,
	}

	var raftPeers []raft.Peer
	cr.peers.ForEach(func(p *Peer) bool {
		if !p.self {
			raftPeers = append(raftPeers, raft.Peer{ID: p.id})
		}
		return true
	})

	// Set peer list to the other nodes in the cluster.
	// Note that they need to be started separately as well.
	node := raft.StartNode(cfg, raftPeers)

	ticker := time.NewTicker(100 * time.Millisecond)
	defer ticker.Stop()

	// event loop on raft state machine updates
	for {
		select {
		case <-ticker.C:
			node.Tick()

		// store raft entries to wal, then publish over commit channel
		case rd := <-node.Ready():
			// wal.Save(rd.HardState, rd.Entries)
			// if !raft.IsEmptySnap(rd.Snapshot) {
			// 	saveSnap(rd.Snapshot)
			// 	raftStorage.ApplySnapshot(rd.Snapshot)
			// 	publishSnapshot(rd.Snapshot)
			// }
			// raftStorage.Append(rd.Entries)
			// transport.Send(rd.Messages)
			// if ok := rc.publishEntries(entriesToApply(rd.CommittedEntries)); !ok {
			// 	stop()
			// 	return
			// }
			// maybeTriggerSnapshot()
			fmt.Println("<<<<<<<<<<<<<<<<<", rd)
			node.Advance()

		// case err := <-rc.transport.ErrorC:
		// 	return

		case <-ctx.Done():
			return
		}
	}
}

func (cr *clusterRaft) init(fnLog log.FnT, peers *Peers) error {
	if fnLog == nil {
		fnLog = log.LogfStd
	}
	cr.fnLog = fnLog

	cr.peers = peers

	return nil
}

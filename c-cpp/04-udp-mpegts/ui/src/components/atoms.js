import React from 'react'
import classNames from 'classnames'
import ak from '../lib/atom-kind'
import h264NALSliceType from '../lib/h264-nal-slice-type'
import humanize from '../helpers/humanize'
import styles from '../styles/atoms.css'

class Atom extends React.Component {
  render() {
    const model = this.props.model

    let cellName = ''
    let cellData = ''

    const classes = {[styles.row]: true}

    if (ak.isH264(model.atomKind)) {
      cellName = `H264 ${ ak.toString(model.atomKind) }`

      if (model.atomKind == ak.H264SPS) {
        // classes[styles['h264-sps']] = true
      } else if (model.atomKind == ak.H264PPS) {
        // classes[styles['h264-pps']] = true
      } else if (model.atomKind == ak.H264AUD) {
        classes[styles['h264-aud']] = true
      } else if (model.atomKind == ak.H264SEI) {
        classes[styles['h264-sei']] = true
      } else if (model.atomKind == ak.H264SliceIDR) {
        cellName = `H264 ${h264NALSliceType.toString(model.sliceType)}`
        cellData = `{\
        size: ${humanize.byte(model.sz)}, \
        frame-num: ${model.frameNum}, \
        pic-order-cnt-lsb: ${model.picOrderCntLsb}\
        }`

        classes[styles['h264-slice-idr']] = true

        if (h264NALSliceType.isI(model.sliceType)) {
          classes[styles.i] = true
        } else if (h264NALSliceType.isP(model.sliceType)) {
          classes[styles.p] = true
        } else if (h264NALSliceType.isB(model.sliceType)) {
          classes[styles.b] = true
        }
      }
    }

    return (
      <div className={classNames(classes)}>
        <div className={classNames(styles.cell, styles.offset)}>
          0x{model.offset.toString(16)}
        </div>
        <div className={classNames(styles.cell, styles.name)}>
          {cellName}
        </div>
        <div className={classNames(styles.cell, styles.data)}>
          {cellData}
        </div>
      </div>
    )
  } // render
} // class

class Atoms extends React.Component {
  createChildComponent(model) {
    const key = model.id
    return <Atom model={model} key={key}/>
  }

  render() {
    const collection = this.props.collection
    const items = collection.map(this.createChildComponent)

    return (
      <div className={styles.grid}>
        {items}
      </div>
    )
  }
}

export default Atoms

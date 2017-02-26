import React from 'react'
import classNames from 'classnames'
import ak from '../lib/atom-kind'
import h264NALSliceType from '../lib/h264-nal-slice-type'

class Atom extends React.Component {
  render() {
    const model = this.props.model

    let cellName = ''
    let cellData = ''

    const classes = {'atoms-grid__row': true}

    if (ak.isH264(model.atomKind)) {
      cellName = `H264 ${ ak.toString(model.atomKind) }`

      if (model.atomKind == ak.H264SPS) {
        classes['atoms-grid__row-h264-sps'] = true
      } else if (model.atomKind == ak.H264PPS) {
        classes['atoms-grid__row-h264-pps'] = true
      } else if (model.atomKind == ak.H264AUD) {
        classes['atoms-grid__row-h264-aud'] = true
      } else if (model.atomKind == ak.H264SEI) {
        classes['atoms-grid__row-h264-sei'] = true
      } else if (model.atomKind == ak.H264SliceIDR) {
        cellName = `H264 ${h264NALSliceType.toString(model.sliceType)}`
        cellData = `{ \
        frame-num: ${model.frameNum}, \
        pic-order-cnt-lsb: ${model.picOrderCntLsb}\
        }`

        classes['atoms-grid__row-h264-slice-idr'] = true

        if (h264NALSliceType.isI(model.sliceType)) {
          classes['atoms-grid__row-h264-slice-idr-i'] = true
        } else if (h264NALSliceType.isP(model.sliceType)) {
          classes['atoms-grid__row-h264-slice-idr-p'] = true
        } else if (h264NALSliceType.isB(model.sliceType)) {
          classes['atoms-grid__row-h264-slice-idr-b'] = true
        }
      }
    }

    return (
      <div className={classNames(classes)}>
        <div className="atoms-grid__cell atoms-grid__cell-offset">
          0x{model.offset.toString(16)}
        </div>
        <div className="atoms-grid__cell atoms-grid_cell-name">
          {cellName}
        </div>
        <div className="atoms-grid__cell atoms-grid_cell-data">
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
      <div className="atoms-grid">
        {items}
      </div>
    )
  }
}

export default Atoms

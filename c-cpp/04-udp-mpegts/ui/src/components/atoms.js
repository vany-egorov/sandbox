import React from 'react'
import classNames from 'classnames'
import ak from '../lib/atom-kind'
import h264NALSliceType from '../lib/h264-nal-slice-type'

class Atom extends React.Component {
  render() {
    const model = this.props.model

    const classes = {
      'atoms-grid__row': true
    }

    switch (model.atomKind) {
    case ak.H264SPS:
      classes['atoms-grid__row_h264-sps'] = true

      return (
        <div className={classNames(classes)}>
          0x{model.offset.toString(16)}&nbsp;
            {model.sz}&nbsp;
            H264 {ak.toString(model.atomKind)}
        </div>
      )
    case ak.H264PPS:
      classes['atoms-grid__row_h264-pps'] = true

      return (
        <div className={classNames(classes)}>
          0x{model.offset.toString(16)}&nbsp;
            {model.sz}&nbsp;
            H264 {ak.toString(model.atomKind)}
        </div>
      )
    case ak.H264AUD:
      classes['atoms-grid__row_h264-aud'] = true

      return (
        <div className={classNames(classes)}>
          0x{model.offset.toString(16)}&nbsp;
            {model.sz}&nbsp;
            H264 {ak.toString(model.atomKind)}
        </div>
      )
    case ak.H264SEI:
      classes['atoms-grid__row_h264-sei'] = true

      return (
        <div className={classNames(classes)}>
          0x{model.offset.toString(16)}&nbsp;
            {model.sz}&nbsp;
            H264 {ak.toString(model.atomKind)}
        </div>
      )
    case ak.H264SliceIDR:
      classes['atoms-grid__row_h264-slice-idr'] = true

      switch (model.sliceType) {
      case h264NALSliceType.I:
      case h264NALSliceType.SI:
      case h264NALSliceType.I2:
      case h264NALSliceType.SI2:
        classes['atoms-grid__row_h264-slice-idr_i'] = true
        break

      case h264NALSliceType.P:
      case h264NALSliceType.SP:
      case h264NALSliceType.P2:
      case h264NALSliceType.SP2:
        classes['atoms-grid__row_h264-slice-idr_p'] = true
        break

      case h264NALSliceType.B:
      case h264NALSliceType.B2:
        classes['atoms-grid__row_h264-slice-idr_b'] = true
        break
      }

      return (
        <div className={classNames(classes)}>
          0x{model.offset.toString(16)}&nbsp;
            {model.sz}&nbsp;
            H264 {h264NALSliceType.toString(model.sliceType)} slice #0 (
            frame-num: {model.frameNum},
            pic-order-cnt-lsb: {model.picOrderCntLsb}
          )
        </div>
      )
    default:
      return (
        <div className={classNames(classes)}>
          0x{model.offset.toString(16)}&nbsp;
            {model.sz}&nbsp;
            H264 {ak.toString(model.atomKind)}
        </div>
      )
    }
  }
}

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

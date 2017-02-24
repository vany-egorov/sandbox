import React from 'react'
import ak from '../lib/atom-kind'

class Atom extends React.Component {
  render() {
    const model = this.props.model

    switch (model.atomKind.v) {
    case ak.H264SliceIDR:
      return (
        <div>
          0x{model.offset.toString(16)}&nbsp;
            {model.size}&nbsp;
            H264 {model.sliceType.toString()} slice #0 (
            frame-num: {model.frameNum},
            pic-order-cnt-lsb: {model.picOrderCntLsb}
          )
        </div>
      )
    default:
      return (
        <div>
          0x{model.offset.toString(16)}&nbsp;
            {model.size}&nbsp;
            H264 {model.atomKind.toString()}
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
      <div>
        {items}
      </div>
    )
  }
}

export default Atoms

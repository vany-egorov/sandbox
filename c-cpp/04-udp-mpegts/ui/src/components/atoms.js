import React from 'react'

class Atom extends React.Component {
  render() {
    const model = this.props.model.atom

    return (
      <div>
        H264 {model.sliceType.toString()} slice #0 (
          frame-num: {model.frameNum},
          pic-order-cnt-lsb: {model.picOrderCntLsb}
        )
      </div>
    )
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

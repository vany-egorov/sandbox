import React from 'react'
import {connect} from 'react-redux'

class Atom extends React.Component {
  render() {
    const model = this.props.model

    return (
      <div>
        H264 {model.toString()} slice
      </div>
    )
  }
}

class Atoms extends React.Component {
  createChildComponent(model, i) {
    return <Atom model={model} key={i}/>
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

function mapStateToProps(state) {
  return {
    collection: state.atoms
  }
}

// function mapDispatchToProps(dispatch) {
//   return {}
// }

export default connect(
  mapStateToProps
  // , mapDispatchToProps
)(Atoms)

import {connect} from 'react-redux'
import Atoms from '../components/atoms'

function mapStateToProps(state) {
  console.log(state.atoms)
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

import React from "react"
import AtomsChart from "./atoms-chart"
import Atoms      from "./atoms"

class App extends React.Component {
  render() {
    return (
      <section>
        <AtomsChart {...this.props}/>
        <Atoms {...this.props}/>
      </section>
    )
  }
}

export default App

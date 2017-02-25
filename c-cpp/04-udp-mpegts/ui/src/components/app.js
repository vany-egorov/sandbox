import React from 'react'
import Chart from './chart'
import Atoms from '../containers/atoms'

const App = () =>
  <section>
    <header className="header">
      <h1 className="header__title header__title_blue">libVA UI</h1>
    </header>

    <Chart />
    <Atoms />

  </section>

export default App

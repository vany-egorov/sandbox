import _ from 'lodash'
import React from 'react'
import * as d3 from 'd3'
import mockData from './atoms-chart-mock-data'
import styles from '../styles/atoms-chart.css'

class Chart {
  constructor(data, selector, margin, ticks) {
    this.data = data
    this.selector = selector
    this.margin = margin
    this.ticks = ticks
  }

  reset() {
    const rect = d3.select(this.selector).node().getBoundingClientRect()

    this.height = rect.height
    this.width = rect.width

    this.szMax = this.getSzMax()

    this.x = this.getX() // scale
    this.y = this.getY() // scale

    this.showEvery = this.getShowEvery()
    this.tickValues = this.getTickValues()
  }

  getShowEvery() {
    if (this.data.length < this.ticks) {
      return 1
    }
    return parseInt(this.data.length/this.ticks)
  }

  getTickValues() {
    return _(this.x.domain())
      .filter((_, i) => {
        return ((i % this.showEvery) == 0)
      })
      .value()
  }

  getX() {
    return d3.scaleBand()
      .domain(_(this.data).map('id').value())
      .range([0, this.width - this.margin.left - this.margin.right])
  }

  getY() {
    return d3.scaleLinear()
      .domain([0, this.szMax])
      .range([this.height - this.margin.top - this.margin.bottom, 0])
  }

  getSzMax() {
    return _(this.data)
      .map('sz')
      .max()
  }

  getXAxis() {
    return d3.axisBottom()
      .scale(this.x)
      .tickSize(10)
      .tickPadding(10)
      .tickValues(this.tickValues)
      .tickFormat((v) => {
        return v
      })
  }

  getYAxis() {
    return d3.axisLeft()
      .scale(this.y)
  }

  drawXAxis() {
    const ty = this.height - this.margin.top - this.margin.bottom
    this.svg
      .append('g')
      .attr('class', 'd3-axis d3-axis-x')
      .attr('transform', `translate(0, ${ ty })`)
      .call(this.getXAxis())
  }

  drawYAxis() {
    this.svg
      .append('g')
      .attr('class', 'd3-axis d3-axis-y d3-axis-y-temp')
      .call(this.getYAxis())
  }

  drawGridLinesX() {
    const lines = this.gGridLines
      .selectAll('.d3-axis-grid-line-x')
      .data(this.tickValues)

    lines.enter()
      .append('line')
      .attr('class', `d3-axis-grid-line ${styles['d3-axis-grid-line-x']}`)
      .attr('x1', (d) => {
        return this.x(d) + this.x.bandwidth()/2 + 0.5
      })
      .attr('x2', (d) => {
        return this.x(d) + this.x.bandwidth()/2 + 0.5
      })
      .attr('y1', 0)
      .attr('y2', this.y(0))
  }

  drawGridLinesY() {
    const lines = this.gGridLines
      .selectAll('.d3-axis-grid-line-y')
      .data(this.y.ticks(10))

    lines.enter()
      .append('line')
      .attr('class', `d3-axis-grid-line ${styles['d3-axis-grid-line-y']}`)
      .attr('x1', 0)
      .attr('x2', this.width - this.margin.left - this.margin.right)
      .attr('y1', (d) => {
        return this.y(d) + 0.5
      })
      .attr('y2', (d) => {
        return this.y(d) + 0.5
      })
  }

  drawRectAtoms() {
    const rect = this.gAtoms
      .selectAll('d3-bar')
      .data(this.data, (d) => {
        return d.id
      })

    rect.enter()
      .append('rect')
      .attr('class', 'd3-bar')
      .attr('x', (d) => {
        return this.x(d.id)
      })
      .attr('y', (d) => {
        return (
          this.height - this.margin.top - this.margin.bottom -
          (this.height - this.margin.top - this.margin.bottom - this.y(d.sz))
        )
      })
      .attr('width', this.x.bandwidth())
      .attr('height', (d) => {
        return this.height - this.margin.top - this.margin.bottom - this.y(d.sz)
      })
  }

  update(data) {
    this.redraw(data)
  }

  redraw() {
    this.reset()

    this.xAxis = this.getXAxis()
    this.svg
      .selectAll('.d3-axis-x')
      .transition().ease('linear')
      .call(this.xAxis())
  }

  getSVG() {
    return d3
      .select(this.selector)
      .append('svg')
      .attr('class', 'chart atoms-chart')
      .attr('width', this.width)
      .attr('height', this.height)
      .append('g')
      .attr(
        'transform',
        `translate(${ this.margin.left }, ${ this.margin.top })`
      )
  }

  getGGridLines() {
    return this.svg
      .append('g')
      .attr('class', 'd3-g d3-g-grid-lines')
  }

  getGAtoms() {
    return this.svg
      .append('g')
      .attr('class', 'd3-g d3-g-atoms')
  }
}

function chart(data, selector) {
  const margin = {
    top: 10,
    right: 40,
    bottom: 40,
    left: 40
  }

  const ticks = 20

  const chart = new Chart(data, selector, margin, ticks)
  chart.reset()
  chart.svg = chart.getSVG()
  chart.gGridLines = chart.getGGridLines()
  chart.gAtoms = chart.getGAtoms()
  chart.drawXAxis()
  chart.drawYAxis()

  chart.drawGridLinesX()
  chart.drawGridLinesY()

  chart.drawRectAtoms()
}

class AtomsChart extends React.Component {
  componentDidMount() {
    chart(mockData, '#atoms-chart')
  }

  render() {
    return (
      <div id="atoms-chart" style={{height: '600px'}}/>
    )
  }
}

export default AtomsChart

import React, { Component } from 'react';
import { Button } from "react-juce";

import ParameterValueStore from './ParameterValueStore';
import type { ParamValue } from './ParamValueType';

interface IProps {
  paramId?: string;
  onToggled?: Function;
  borderColor?: string;
}

interface IState {
  defaultValue?: ParamValue;
  value?: ParamValue;
}

// Extend NodeJS.Global
// https://stackoverflow.com/questions/40743131/how-to-prevent-property-does-not-exist-on-type-global-with-jsdom-and-t#answer-42304473
declare global {
  namespace NodeJS {
    interface Global {
      setParameterValueNotifyingHost: Function;
      beginParameterChangeGesture: Function;
      endParameterChangeGesture: Function;
    }
  }
}

class ParameterToggleButton extends Component<IProps, IState> {
  constructor(props: IProps) {
    super(props);

    this._handleClick = this._handleClick.bind(this);
    this._onParameterValueChange = this._onParameterValueChange.bind(this);

    const paramState = ParameterValueStore.getParameterState(this.props.paramId);

    const initialDefaultValue = typeof paramState.defaultValue === 'number' ?
      paramState.defaultValue : 0.0;

    const initialValue = typeof paramState.currentValue === 'number' ?
      paramState.currentValue : 0.0;

    this.state = {
      defaultValue: initialDefaultValue,
      value: initialValue,
    };

    if (typeof this.props.onToggled === 'function') {
      this.props.onToggled(initialValue !== 0.0);
    }
  }

  componentDidMount() {
    ParameterValueStore.addListener(
      ParameterValueStore.CHANGE_EVENT,
      this._onParameterValueChange
    );
  }

  componentWillUnmount() {
    ParameterValueStore.removeListener(
      ParameterValueStore.CHANGE_EVENT,
      this._onParameterValueChange
    );
  }

  _handleClick(_e: any) {
    const newValue = this.state.value === 0.0 ? 1.0 : 0.0

    this.setState({
      value: newValue
    });

    if (typeof this.props.paramId === 'string' && this.props.paramId.length > 0) {
      global.beginParameterChangeGesture(this.props.paramId);
      global.setParameterValueNotifyingHost(this.props.paramId, newValue);
      global.endParameterChangeGesture(this.props.paramId);
    }

    if (typeof this.props.onToggled === 'function') {
      this.props.onToggled(newValue !== 0.0);
    }
  }

  _onParameterValueChange(paramId: string) {
    const shouldUpdate = typeof this.props.paramId === 'string' &&
      this.props.paramId.length > 0 &&
      this.props.paramId === paramId;

    if (shouldUpdate) {
      const state = ParameterValueStore.getParameterState(paramId);

      const newDefaultValue = state.defaultValue;
      const newValue = state.currentValue;

      this.setState({
        defaultValue: newDefaultValue,
        value: newValue,
      });

      if (typeof this.props.onToggled === 'function') {
        this.props.onToggled(newValue !== 0.0);
      }
    }
  }

  render() {
    const { paramId, onToggled, ...other } = this.props;

    return (
      <Button {...other} onClick={this._handleClick}>
        {this.props.children}
      </Button>
    )
  }
}

export default ParameterToggleButton;

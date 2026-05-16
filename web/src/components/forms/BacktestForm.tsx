import type { BacktestFormValues, StrategyId } from "../../types/backtest";
import TickerSelector from "./TickerSelector";

type Props = {
  strategy: StrategyId;
  values: BacktestFormValues;
  isRunning: boolean;
  submitLabel: string;
  runningLabel: string;
  error: string | null;
  onChange: (values: BacktestFormValues) => void;
  onSubmit: () => void;
};

function BacktestForm({
  strategy,
  values,
  isRunning,
  submitLabel,
  runningLabel,
  error,
  onChange,
  onSubmit,
}: Props) {
  function updateValues(nextValues: Partial<BacktestFormValues>) {
    onChange({ ...values, ...nextValues });
  }

  return (
    <form
      className="panel backtest-form"
      onSubmit={(event) => {
        event.preventDefault();
        onSubmit();
      }}
    >
      <div className="form-row form-row-wide">
        <label htmlFor={`${strategy}-tickers`}>Tickers</label>
        <TickerSelector
          selectedSymbols={values.selectedTickers}
          onChange={(selectedTickers) => updateValues({ selectedTickers })}
        />
      </div>

      <div className="form-row">
        <label htmlFor={`${strategy}-start-date`}>Start date</label>
        <input
          id={`${strategy}-start-date`}
          type="date"
          value={values.start}
          onChange={(event) => updateValues({ start: event.target.value })}
        />
      </div>

      <div className="form-row">
        <label htmlFor={`${strategy}-end-date`}>End date</label>
        <input
          id={`${strategy}-end-date`}
          type="date"
          value={values.end}
          onChange={(event) => updateValues({ end: event.target.value })}
        />
      </div>

      <div className={strategy === "random" ? "form-row" : "form-row form-row-wide"}>
        <label htmlFor={`${strategy}-initial-cash`}>Initial cash</label>
        <input
          id={`${strategy}-initial-cash`}
          type="number"
          min="1"
          value={values.initialCash}
          onChange={(event) =>
            updateValues({ initialCash: Number(event.target.value) })
          }
        />
      </div>

      {strategy === "random" && (
        <>
          <div className="form-row">
            <label htmlFor="random-target-notional">Target notional</label>
            <input
              id="random-target-notional"
              type="number"
              min="1"
              value={values.targetNotional}
              onChange={(event) =>
                updateValues({ targetNotional: Number(event.target.value) })
              }
            />
          </div>

          <div className="form-row">
            <label htmlFor="random-seed">Seed</label>
            <input
              id="random-seed"
              type="number"
              value={values.seed}
              onChange={(event) => updateValues({ seed: Number(event.target.value) })}
            />
          </div>
        </>
      )}

      <button className="primary-button" type="submit" disabled={isRunning}>
        {isRunning ? runningLabel : submitLabel}
      </button>

      {error && <p className="error-message">Error: {error}</p>}
    </form>
  );
}

export default BacktestForm;

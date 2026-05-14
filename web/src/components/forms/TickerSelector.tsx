import { useMemo, useState } from "react";
import { SUPPORTED_TICKERS } from "../../data/supportedTickers";

type Props = {
  selectedSymbols: string[];
  onChange: (symbols: string[]) => void;
};

function TickerSelector({ selectedSymbols, onChange }: Props) {
  const [query, setQuery] = useState("");
  const selectedSet = new Set(selectedSymbols);

  const selectedTickers = selectedSymbols
    .map((symbol) => SUPPORTED_TICKERS.find((ticker) => ticker.symbol === symbol))
    .filter(Boolean);

  const filteredTickers = useMemo(() => {
    const normalizedQuery = query.trim().toLowerCase();

    if (normalizedQuery.length === 0) {
      return SUPPORTED_TICKERS;
    }

    return SUPPORTED_TICKERS.filter((ticker) => {
      const searchableText = [
        ticker.symbol,
        ticker.name,
        ticker.market,
        ticker.sector ?? "",
      ]
        .join(" ")
        .toLowerCase();

      return searchableText.includes(normalizedQuery);
    });
  }, [query]);

  function toggleSymbol(symbol: string) {
    if (selectedSet.has(symbol)) {
      onChange(selectedSymbols.filter((selectedSymbol) => selectedSymbol !== symbol));
      return;
    }

    onChange([...selectedSymbols, symbol]);
  }

  return (
    <div className="ticker-selector">
      <div className="selected-ticker-list" aria-label="Selected tickers">
        {selectedTickers.length > 0 ? (
          selectedTickers.map((ticker) =>
            ticker ? (
              <button
                className="selected-ticker-pill"
                key={ticker.symbol}
                type="button"
                onClick={() => toggleSymbol(ticker.symbol)}
              >
                <span>{ticker.symbol}</span>
                <span aria-hidden="true">x</span>
              </button>
            ) : null
          )
        ) : (
          <span className="selected-ticker-empty">No tickers selected</span>
        )}
      </div>

      <input
        aria-label="Search tickers"
        className="ticker-search"
        placeholder="Search by company, ticker, market, or sector"
        value={query}
        onChange={(event) => setQuery(event.target.value)}
      />

      <div className="ticker-option-list">
        {filteredTickers.map((ticker) => {
          const isSelected = selectedSet.has(ticker.symbol);

          return (
            <button
              className={
                isSelected
                  ? "ticker-option ticker-option-selected"
                  : "ticker-option"
              }
              key={ticker.symbol}
              type="button"
              onClick={() => toggleSymbol(ticker.symbol)}
            >
              <span>
                <strong>{ticker.name}</strong>
                <small>{ticker.sector}</small>
              </span>
              <span className="ticker-option-meta">
                <strong>{ticker.symbol}</strong>
                <small>{ticker.market}</small>
              </span>
            </button>
          );
        })}
      </div>
    </div>
  );
}

export default TickerSelector;

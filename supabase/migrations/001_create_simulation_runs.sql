create table if not exists public.simulation_runs (
  id uuid primary key default gen_random_uuid(),
  created_at timestamptz not null default now(),
  name text not null,
  dataset_id text not null,
  symbols text[] not null,
  initial_cash numeric not null
);

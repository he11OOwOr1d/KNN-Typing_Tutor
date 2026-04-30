"use client";

import { Activity, ArrowLeft } from "lucide-react";
import Link from "next/link";
import { TelemetryDashboard } from "@/components/TelemetryDashboard";
import { Spotlight } from "@/components/ui/spotlight";

export default function AnalyticsPage() {
  return (
    <main className="relative min-h-screen overflow-x-hidden bg-[#0a0a0a]">
      <Spotlight />
      <div aria-hidden="true" className="typing-grid absolute inset-0" />
      <div aria-hidden="true" className="noise pointer-events-none absolute inset-0 opacity-40" />

      <div className="relative z-10 mx-auto flex w-full max-w-7xl flex-col px-4 py-5 sm:px-6 lg:px-8">
        <header className="sticky top-0 z-20 -mx-4 flex items-center justify-between border-b border-border/80 bg-background/82 px-4 pb-4 pt-2 backdrop-blur sm:-mx-6 sm:px-6 lg:-mx-8 lg:px-8">
          <div className="flex items-center gap-4">
            <Link href="/" className="flex items-center justify-center rounded-full bg-white/5 p-2 transition-colors hover:bg-white/10 text-muted-foreground hover:text-white">
              <ArrowLeft className="h-5 w-5" />
            </Link>
            <div className="scale-panel flex h-10 w-10 items-center justify-center rounded-sm border border-amber-500/30 shadow-sm bg-amber-500/10">
              <Activity className="h-5 w-5 text-amber-500" />
            </div>
            <h1 className="ui-title text-xl font-semibold text-foreground sm:text-2xl">Fatigue Analytics</h1>
          </div>
          
          <nav className="hidden items-center gap-6 font-mono text-[11px] uppercase tracking-[0.18em] text-muted-foreground lg:flex">
            <Link href="/" className="hover:text-foreground transition-colors">Home</Link>
            <Link href="/race" className="hover:text-foreground transition-colors">Race Mode</Link>
            <span className="text-foreground">Analytics</span>
          </nav>
        </header>

        <section className="flex flex-1 flex-col py-8">
          <div className="mb-8">
            <p className="mb-3 font-mono text-[11px] uppercase tracking-[0.22em] text-muted-foreground">
              Deep dive into your performance metrics
            </p>
            <h2 className="ui-title max-w-3xl text-3xl font-semibold text-foreground sm:text-5xl">
              Telemetry & Fatigue
            </h2>
          </div>
          
          <TelemetryDashboard />
        </section>
      </div>
    </main>
  );
}

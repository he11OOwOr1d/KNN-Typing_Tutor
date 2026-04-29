import Link from "next/link";
import { Activity, Zap } from "lucide-react";
import { Spotlight } from "@/components/ui/spotlight";

export default function Home() {
  return (
    <main className="relative flex min-h-screen flex-col items-center justify-center overflow-x-hidden bg-[#0a0a0a]">
      <Spotlight />
      <div aria-hidden="true" className="typing-grid absolute inset-0" />
      <div aria-hidden="true" className="noise pointer-events-none absolute inset-0 opacity-40" />

      <div className="relative z-10 mx-auto flex w-full max-w-4xl flex-col items-center px-4 text-center sm:px-6 lg:px-8">
        <div className="mb-6 flex h-16 w-16 items-center justify-center rounded-2xl border border-white/10 bg-black/50 shadow-2xl backdrop-blur-xl">
          <Zap className="h-8 w-8 text-cyan-400 drop-shadow-[0_0_15px_rgba(34,211,238,0.8)]" />
        </div>

        <h1 className="ui-title mb-4 text-5xl font-bold tracking-tight text-white sm:text-7xl">
          KNN <span className="text-cyan-400">Type</span>
        </h1>

        <p className="mb-12 max-w-2xl text-lg text-muted-foreground sm:text-xl">
          Master your typing rhythm. Race against your ghost, track every keystroke, and analyze your fatigue curve in real-time.
        </p>

        <div className="grid w-full gap-6 sm:grid-cols-2 sm:px-12">
          <Link
            href="/race"
            className="group relative flex flex-col items-center gap-4 rounded-2xl border border-white/10 bg-black/20 p-8 text-center backdrop-blur-md transition-all hover:border-cyan-500/50 hover:bg-black/40 hover:shadow-[0_0_40px_rgba(34,211,238,0.15)]"
          >
            <div className="rounded-full bg-cyan-500/10 p-4 transition-transform group-hover:scale-110 group-hover:bg-cyan-500/20">
              <Zap className="h-8 w-8 text-cyan-400" />
            </div>
            <div>
              <h2 className="text-2xl font-semibold text-white">Start Racing</h2>
              <p className="mt-2 text-sm text-muted-foreground">Jump into the arena, race your ghost, and set new high scores.</p>
            </div>
          </Link>

          <Link
            href="/analytics"
            className="group relative flex flex-col items-center gap-4 rounded-2xl border border-white/10 bg-black/20 p-8 text-center backdrop-blur-md transition-all hover:border-amber-500/50 hover:bg-black/40 hover:shadow-[0_0_40px_rgba(245,158,11,0.15)]"
          >
            <div className="rounded-full bg-amber-500/10 p-4 transition-transform group-hover:scale-110 group-hover:bg-amber-500/20">
              <Activity className="h-8 w-8 text-amber-400" />
            </div>
            <div>
              <h2 className="text-2xl font-semibold text-white">Analytics &amp; Fatigue</h2>
              <p className="mt-2 text-sm text-muted-foreground">Dive deep into your performance data and fatigue predictions.</p>
            </div>
          </Link>
        </div>
      </div>
    </main>
  );
}

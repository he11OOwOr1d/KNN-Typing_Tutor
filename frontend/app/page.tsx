"use client";

import { useCallback, useEffect, useMemo, useRef, useState } from "react";
import { AnimatePresence, motion } from "framer-motion";
import type { ReactNode } from "react";
import { Activity, ArrowRight, Flag, Gauge, Ghost, Keyboard, RotateCcw, TimerReset, Trophy, Zap } from "lucide-react";
import { Badge } from "@/components/ui/badge";
import { Button } from "@/components/ui/button";
import { TelemetryDashboard } from "@/components/TelemetryDashboard";
import { Card, CardContent, CardTitle } from "@/components/ui/card";
import { Progress } from "@/components/ui/progress";
import { Spotlight } from "@/components/ui/spotlight";
import { cn } from "@/lib/utils";

type Sentence = {
  id: string;
  text: string;
};

type GhostAttempt = {
  id: string;
  sentenceId: string;
  text: string;
  typed: string;
  timestamps: number[];
  durationMs: number;
  stats: {
    accuracy: number;
    wpm: number;
  };
};

const apiBase = process.env.NEXT_PUBLIC_API_URL || "http://localhost:4000";
const minRecordedKeyIntervalMs = 32;
const timerOptions = [15, 30, 60, 90];

function calculateStats(target: string, typed: string, elapsedMs: number) {
  const correct = [...typed].filter((char, index) => char === target[index]).length;
  const accuracy = typed.length ? Math.round((correct / typed.length) * 100) : 100;
  const measuredMs = Math.max(elapsedMs, typed.length * minRecordedKeyIntervalMs);
  const minutes = Math.max(measuredMs / 60000, 1 / 60);
  const wpm = Math.round(correct / 5 / minutes);

  return { accuracy, wpm, correct };
}

function formatTime(ms: number) {
  return `${(ms / 1000).toFixed(1)}s`;
}

function formatTimer(ms: number) {
  return `${Math.max(Math.ceil(ms / 1000), 0)}s`;
}

function buildWordGlyphs(target: string) {
  let index = 0;

  return target.split(" ").map((word, wordIndex, words) => {
    const chars = [...word].map((char) => ({ char, index: index++ }));
    const space = wordIndex < words.length - 1 ? { char: " ", index: index++ } : null;

    return { chars, space, key: `${word}-${wordIndex}` };
  });
}

function normalizeGhostAttempt(attempt: GhostAttempt | null): GhostAttempt | null {
  if (!attempt) {
    return null;
  }

  const timestamps = attempt.timestamps.reduce<number[]>((normalized, time, index) => {
    const previous = normalized[normalized.length - 1] ?? 0;
    normalized.push(Math.max(Number(time) || 0, previous + minRecordedKeyIntervalMs, (index + 1) * minRecordedKeyIntervalMs));
    return normalized;
  }, []);
  const durationMs = Math.max(attempt.durationMs, timestamps[timestamps.length - 1] || 0);
  const stats = calculateStats(attempt.text, attempt.typed, durationMs);

  return {
    ...attempt,
    timestamps,
    durationMs,
    stats: {
      accuracy: stats.accuracy,
      wpm: stats.wpm
    }
  };
}

export default function Home() {
  const [sentences, setSentences] = useState<Sentence[]>([]);
  const [sentenceIndex, setSentenceIndex] = useState(0);
  const [typed, setTyped] = useState("");
  const [timestamps, setTimestamps] = useState<number[]>([]);
  const [startedAt, setStartedAt] = useState<number | null>(null);
  const [elapsedMs, setElapsedMs] = useState(0);
  const [ghost, setGhost] = useState<GhostAttempt | null>(null);
  const [ghostIndex, setGhostIndex] = useState(-1);
  const [savedAttempt, setSavedAttempt] = useState<GhostAttempt | null>(null);
  const [isSaving, setIsSaving] = useState(false);
  const [timerSeconds, setTimerSeconds] = useState(60);
  const [keystrokes, setKeystrokes] = useState<{key: string, isError: boolean}[]>([]);
  const [showTelemetry, setShowTelemetry] = useState(false);
  const inputRef = useRef<HTMLInputElement>(null);
  const saveLockRef = useRef(false);

  const sentence = sentences[sentenceIndex];
  const target = sentence?.text || "";
  const timeLimitMs = timerSeconds * 1000;
  const isFinished = typed.length === target.length && target.length > 0;
  const isTimeUp = Boolean(startedAt && elapsedMs >= timeLimitMs && !isFinished);
  const isRunOver = isFinished || isTimeUp;
  const isPerfect = typed === target && target.length > 0;
  const measuredElapsedMs = Math.min(elapsedMs, timeLimitMs);
  const remainingMs = startedAt ? Math.max(timeLimitMs - measuredElapsedMs, 0) : timeLimitMs;
  const stats = useMemo(() => calculateStats(target, typed, measuredElapsedMs), [measuredElapsedMs, target, typed]);
  const progress = target ? Math.round((typed.length / target.length) * 100) : 0;
  const wordGlyphs = useMemo(() => buildWordGlyphs(target), [target]);

  const loadGhost = useCallback(async (sentenceId: string) => {
    const response = await fetch(`${apiBase}/api/sentences/${sentenceId}/ghost`);
    const data = await response.json();
    setGhost(normalizeGhostAttempt(data.ghost));
  }, []);

  const resetRun = useCallback(
    (nextIndex = sentenceIndex) => {
      const nextSentence = sentences[nextIndex];
      setSentenceIndex(nextIndex);
      setTyped("");
      setTimestamps([]);
      setKeystrokes([]);
      setStartedAt(null);
      setElapsedMs(0);
      setGhostIndex(-1);
      setSavedAttempt(null);
      saveLockRef.current = false;
      if (nextSentence) {
        void loadGhost(nextSentence.id);
      }
      window.setTimeout(() => inputRef.current?.focus(), 0);
    },
    [loadGhost, sentenceIndex, sentences]
  );

  useEffect(() => {
    async function loadSentences() {
      const response = await fetch(`${apiBase}/api/sentences`);
      const data = await response.json();
      setSentences(data.sentences);
      if (data.sentences[0]) {
        await loadGhost(data.sentences[0].id);
      }
    }

    void loadSentences();
  }, [loadGhost]);

  useEffect(() => {
    if (!startedAt || isRunOver) {
      return;
    }

    const interval = window.setInterval(() => {
      setElapsedMs(Math.min(performance.now() - startedAt, timeLimitMs));
    }, 50);

    return () => window.clearInterval(interval);
  }, [isRunOver, startedAt, timeLimitMs]);

  useEffect(() => {
    if (!startedAt || !ghost?.timestamps.length || isRunOver) {
      return;
    }

    const interval = window.setInterval(() => {
      const now = performance.now() - startedAt;
      const nextIndex = ghost.timestamps.findLastIndex((time) => time <= now);
      setGhostIndex(nextIndex);
    }, 24);

    return () => window.clearInterval(interval);
  }, [ghost, isRunOver, startedAt]);

  useEffect(() => {
    async function saveAttempt() {
      if (!sentence || saveLockRef.current || !isFinished) {
        return;
      }

      saveLockRef.current = true;
      setIsSaving(true);

      const duration = Math.max(timestamps[timestamps.length - 1] || measuredElapsedMs, typed.length * minRecordedKeyIntervalMs);
      const response = await fetch(`${apiBase}/api/attempts`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({
          sentenceId: sentence.id,
          typed,
          timestamps,
          durationMs: duration,
          keystrokes
        })
      });

      const data = await response.json();
      setSavedAttempt(data.attempt);
      setIsSaving(false);
      await loadGhost(sentence.id);
    }

    void saveAttempt();
  }, [isFinished, loadGhost, measuredElapsedMs, sentence, timestamps, typed]);

  function handleInput(rawValue: string) {
    if (!target || isRunOver) {
      return;
    }

    const value =
      rawValue.length > typed.length + 1
        ? `${typed}${rawValue.slice(typed.length, typed.length + 1)}`
        : rawValue.slice(0, target.length);

    const now = performance.now();
    const runStart = startedAt ?? now;
    if (startedAt && now - runStart >= timeLimitMs) {
      setElapsedMs(timeLimitMs);
      return;
    }

    const elapsed = Math.min(now - runStart, timeLimitMs);

    if (!startedAt) {
      setStartedAt(runStart);
    }

    if (value.length >= typed.length) {
      const added = value.length - typed.length;
      if (added > 0) {
        setTimestamps((current) => {
          const previous = current[current.length - 1] ?? 0;
          const recorded = Math.max(elapsed, previous + minRecordedKeyIntervalMs);
          return [...current, ...Array.from({ length: added }, () => recorded)];
        });
      }
    } else {
      setTimestamps((current) => current.slice(0, value.length));
    }

    setTyped(value);
    setElapsedMs(elapsed);
  }

  const handleKeyDown = (event: React.KeyboardEvent<HTMLInputElement>) => {
    if (!target || isRunOver || event.ctrlKey || event.metaKey || event.altKey || event.key.length !== 1) {
      return;
    }
    
    const expectedChar = target[typed.length];
    const isError = event.key !== expectedChar;
    
    setKeystrokes((prev) => [
      ...prev,
      {
        key: event.key.toLowerCase(),
        isError
      }
    ]);
  };

  const currentIndex = typed.length;
  const activeIndex = Math.min(currentIndex, Math.max(target.length - 1, 0));
  const ghostSafeIndex = Math.min(Math.max(ghostIndex, -1), Math.max(target.length - 1, 0));
  const ghostProgress = target ? Math.round(((ghostSafeIndex + 1) / target.length) * 100) : 0;
  const racerDelta = ghostIndex >= 0 ? typed.length - (ghostSafeIndex + 1) : typed.length;

  function selectTimer(nextTimerSeconds: number) {
    setTimerSeconds(nextTimerSeconds);
    resetRun();
  }

  return (
    <main className="relative min-h-screen overflow-x-hidden">
      <Spotlight />
      <div aria-hidden="true" className="typing-grid absolute inset-0" />
      <div aria-hidden="true" className="noise pointer-events-none absolute inset-0 opacity-40" />

      <div className="relative mx-auto flex min-h-screen w-full max-w-7xl flex-col px-4 py-5 sm:px-6 lg:px-8">
        <header className="sticky top-0 z-20 -mx-4 flex items-center justify-between gap-4 border-b border-border/80 bg-background/82 px-4 pb-4 pt-1 backdrop-blur sm:-mx-6 sm:px-6 lg:-mx-8 lg:px-8">
          <div className="flex items-center gap-3">
            <div className="scale-panel flex h-10 w-10 items-center justify-center rounded-sm border border-border shadow-sm">
              <Zap className="h-5 w-5 text-primary" />
            </div>
            <div>
              <h1 className="ui-title text-xl font-semibold text-foreground sm:text-2xl">KNN Type</h1>
              <div className="mt-1 flex items-center gap-2">
                <Badge>ghost race</Badge>
                <span className="font-mono text-xs text-muted-foreground">{sentence?.id || "loading"}</span>
              </div>
            </div>
          </div>

          <nav className="hidden items-center gap-6 font-mono text-[11px] uppercase tracking-[0.18em] text-muted-foreground lg:flex">
            <button onClick={() => setShowTelemetry(false)} className={!showTelemetry ? "text-foreground" : "hover:text-foreground transition-colors"}>Benchmark</button>
            <span>Rhythm</span>
            <button onClick={() => setShowTelemetry(true)} className={showTelemetry ? "text-foreground" : "hover:text-foreground transition-colors"}>Telemetry</button>
          </nav>

          <div className="flex items-center gap-2">
            <Button variant="secondary" size="icon" aria-label="Reset" onClick={() => resetRun()}>
              <RotateCcw className="h-4 w-4" />
            </Button>
            <Button
              size="icon"
              aria-label="New line"
              onClick={() => resetRun((sentenceIndex + 1) % Math.max(sentences.length, 1))}
              disabled={!sentences.length}
            >
              <ArrowRight className="h-4 w-4" />
            </Button>
          </div>
        </header>

        <section className="flex flex-1 flex-col gap-5 py-5">
          {showTelemetry ? (
            <div className="py-3 sm:py-7">
              <div className="mb-4 h-px w-full scale-line" />
              <div className="mb-8">
                <p className="mb-3 font-mono text-[11px] uppercase tracking-[0.22em] text-muted-foreground">
                  Historical analysis & patterns
                </p>
                <h2 className="ui-title max-w-3xl text-3xl font-semibold text-foreground sm:text-6xl">
                  Telemetry Dashboard
                </h2>
              </div>
              <TelemetryDashboard />
            </div>
          ) : (
            <>
            <div className="py-3 sm:py-7">
              <div className="mb-4 h-px w-full scale-line" />
              <div className="flex flex-col justify-between gap-4 md:flex-row md:items-end">
                <div>
                  <p className="mb-3 font-mono text-[11px] uppercase tracking-[0.22em] text-muted-foreground">
                    Evaluation run / live keystroke telemetry
                  </p>
                  <h2 className="ui-title max-w-3xl text-3xl font-semibold text-foreground sm:text-6xl">
                    Race the rhythm behind your last run.
                  </h2>
                </div>
                <div className="grid grid-cols-2 gap-2 font-mono text-xs text-muted-foreground">
                  <span>Mode</span>
                  <span className="text-right text-foreground">Sentence</span>
                  <span>Storage</span>
                  <span className="text-right text-foreground">Local JSON</span>
                </div>
              </div>
            </div>

            <div className="grid grid-cols-2 gap-3 sm:grid-cols-4">
              <Stat icon={<Gauge className="h-4 w-4" />} label="wpm" value={stats.wpm} />
              <Stat icon={<Keyboard className="h-4 w-4" />} label="acc" value={`${stats.accuracy}%`} />
              <Stat icon={<TimerReset className="h-4 w-4" />} label="left" value={formatTimer(remainingMs)} />
              <Stat icon={<Activity className="h-4 w-4" />} label="ghost" value={ghost ? formatTime(ghost.durationMs) : "--"} />
            </div>

            <div className="timer-selector scale-panel flex flex-col gap-3 rounded-lg border border-white/10 p-3 shadow-sm backdrop-blur sm:flex-row sm:items-center sm:justify-between">
              <div className="flex items-center gap-2 font-mono text-[11px] uppercase tracking-[0.16em] text-muted-foreground">
                <TimerReset className="h-4 w-4 text-cyan-100" />
                Timer
              </div>
              <div className="grid grid-cols-4 gap-1 rounded-md border border-white/10 bg-black/15 p-1">
                {timerOptions.map((option) => (
                  <button
                    key={option}
                    type="button"
                    className={cn(
                      "rounded px-3 py-1.5 font-mono text-xs font-semibold transition",
                      timerSeconds === option
                        ? "bg-cyan-100 text-slate-950 shadow-[0_0_20px_rgba(103,232,249,0.22)]"
                        : "text-muted-foreground hover:bg-white/10 hover:text-foreground"
                    )}
                    onClick={() => selectTimer(option)}
                  >
                    {option}s
                  </button>
                ))}
              </div>
            </div>

            <div className="grid items-start gap-5 lg:grid-cols-[1fr_340px]">
              <div className="space-y-5">
                <button
                  className="typing-pad scale-panel group relative w-full overflow-hidden rounded-lg border border-white/10 p-4 text-left shadow-[0_28px_90px_rgba(0,0,0,0.34)] outline-none ring-ring backdrop-blur transition focus-visible:ring-2 sm:p-7"
                  onClick={() => inputRef.current?.focus()}
                >
                  <div className="pointer-events-none absolute inset-x-0 top-0 h-px bg-gradient-to-r from-transparent via-white/30 to-transparent" />
                  <input
                    ref={inputRef}
                    value={typed}
                    onChange={(event) => handleInput(event.target.value)}
                    onKeyDown={handleKeyDown}
                    onPaste={(event) => event.preventDefault()}
                    className="sr-only"
                    autoFocus
                    aria-label="Typing input"
                  />

                  <div className="mb-8 flex items-center gap-4">
                    <Progress className="h-2 rounded-full bg-white/[0.08]" value={progress} />
                    <span className="min-w-10 text-right font-mono text-[11px] font-semibold text-cyan-100">{progress}%</span>
                  </div>

                  <div className="relative min-h-[220px] sm:min-h-[240px]">
                    <div className="typing-copy flex flex-wrap gap-y-4 text-[1.32rem] leading-[1.74] sm:text-[clamp(1.72rem,2.45vw,2.25rem)]">
                      {wordGlyphs.map((word) => (
                        <span key={word.key} className="mr-[0.56em] inline-flex flex-nowrap">
                          {word.chars.map((glyph) => (
                            <Glyph
                              key={`${glyph.char}-${glyph.index}`}
                              char={glyph.char}
                              index={glyph.index}
                              typed={typed}
                              activeIndex={activeIndex}
                              ghostIndex={ghostSafeIndex}
                              hasGhost={ghostIndex >= 0}
                              isFinished={isFinished}
                            />
                          ))}
                          {word.space ? (
                            <Glyph
                              key={`space-${word.space.index}`}
                              char={word.space.char}
                              index={word.space.index}
                              typed={typed}
                              activeIndex={activeIndex}
                              ghostIndex={ghostSafeIndex}
                              hasGhost={ghostIndex >= 0}
                              isFinished={isFinished}
                              isSpace
                            />
                          ) : null}
                        </span>
                      ))}
                    </div>
                  </div>
                </button>

                <AnimatePresence>
                  {isRunOver ? (
                    <ResultStrip
                      isPerfect={isPerfect}
                      isTimeUp={isTimeUp}
                      isSaving={isSaving}
                      savedAttempt={savedAttempt}
                      stats={stats}
                      onNext={() => resetRun((sentenceIndex + 1) % Math.max(sentences.length, 1))}
                    />
                  ) : null}
                </AnimatePresence>
              </div>

              <aside>
                <RaceStatusPanel
                  ghost={ghost}
                  ghostProgress={ghostProgress}
                  progress={progress}
                  racerDelta={racerDelta}
                  stats={stats}
                  typedCount={typed.length}
                  targetCount={target.length}
                  isLive={Boolean(startedAt && !isRunOver)}
                  timerSeconds={timerSeconds}
                />
              </aside>
            </div>
            </>
          )}
        </section>
      </div>
    </main>
  );
}

function Glyph({
  char,
  index,
  typed,
  activeIndex,
  ghostIndex,
  hasGhost,
  isFinished,
  isSpace = false
}: {
  char: string;
  index: number;
  typed: string;
  activeIndex: number;
  ghostIndex: number;
  hasGhost: boolean;
  isFinished: boolean;
  isSpace?: boolean;
}) {
  const typedChar = typed[index];
  const isCurrent = index === activeIndex && !isFinished;
  const isGhost = index === ghostIndex && hasGhost && !isFinished;
  const isCorrect = typedChar === char;
  const isTyped = index < typed.length;

  return (
    <span className={cn("relative inline-block whitespace-pre", isSpace ? "w-[0.2em]" : "")}>
      {isGhost ? (
        <span className={cn("ghost-inline-marker", isSpace ? "ghost-inline-marker-space" : "")} aria-hidden="true">
          <span className="ghost-inline-eye" />
        </span>
      ) : null}
      {isCurrent ? (
        <span className="caret-pulse absolute -left-0.5 top-[10%] h-[80%] w-px bg-primary shadow-[0_0_20px_rgba(255,255,255,0.45)]" />
      ) : null}
      <span
        className={cn(
          "transition-colors",
          !isTyped && "text-muted-foreground/45",
          isTyped && isCorrect && "text-foreground",
          isTyped && !isCorrect && "bg-destructive/14 text-destructive"
        )}
      >
        {char}
      </span>
    </span>
  );
}

function ResultStrip({
  isPerfect,
  isTimeUp,
  isSaving,
  savedAttempt,
  stats,
  onNext
}: {
  isPerfect: boolean;
  isTimeUp: boolean;
  isSaving: boolean;
  savedAttempt: GhostAttempt | null;
  stats: ReturnType<typeof calculateStats>;
  onNext: () => void;
}) {
  const status = isTimeUp ? "Time up" : isSaving ? "Saving run" : savedAttempt ? "Run saved" : "Run complete";

  return (
    <motion.div
      initial={{ opacity: 0, y: 8 }}
      animate={{ opacity: 1, y: 0 }}
      exit={{ opacity: 0, y: 8 }}
      className={cn(
        "result-strip flex flex-col gap-3 rounded-lg border px-4 py-3 shadow-sm sm:flex-row sm:items-center sm:justify-between",
        isPerfect ? "border-emerald-200/20" : "border-amber-200/24"
      )}
    >
      <div className="flex items-center gap-3">
        <span className={cn("h-2.5 w-2.5 rounded-full", isPerfect ? "bg-emerald-300" : "bg-amber-300")} />
        <div>
          <div className="text-sm font-semibold text-foreground">{status}</div>
          <div className="font-mono text-[11px] uppercase tracking-[0.14em] text-muted-foreground">
            {isTimeUp ? "timed result" : isPerfect ? "clean finish" : "finish recorded"}
          </div>
        </div>
      </div>
      <div className="flex items-center justify-between gap-4 sm:justify-end">
        <div className="font-mono text-xs text-muted-foreground">
          <span className="text-foreground">{stats.wpm}</span> wpm /{" "}
          <span className="text-foreground">{stats.accuracy}%</span>
        </div>
        <Button size="sm" onClick={onNext}>
          <ArrowRight className="h-4 w-4" />
          Next
        </Button>
      </div>
    </motion.div>
  );
}

function Stat({ icon, label, value }: { icon: ReactNode; label: string; value: ReactNode }) {
  return (
    <div className="scale-panel rounded-lg border border-white/10 p-3 shadow-sm backdrop-blur">
      <div className="mb-3 flex items-center gap-2 font-mono text-[11px] uppercase tracking-[0.18em] text-muted-foreground">
        {icon}
        {label}
      </div>
      <div className="ui-title text-2xl font-semibold text-foreground">{value}</div>
    </div>
  );
}

function RaceStatusPanel({
  ghost,
  ghostProgress,
  progress,
  racerDelta,
  stats,
  typedCount,
  targetCount,
  isLive,
  timerSeconds
}: {
  ghost: GhostAttempt | null;
  ghostProgress: number;
  progress: number;
  racerDelta: number;
  stats: ReturnType<typeof calculateStats>;
  typedCount: number;
  targetCount: number;
  isLive: boolean;
  timerSeconds: number;
}) {
  const safeGhostProgress = Math.min(Math.max(ghostProgress, 0), 100);
  const safeProgress = Math.min(Math.max(progress, 0), 100);
  const runnerPosition = Math.min(Math.max(safeGhostProgress, 6), 94);
  const leadLabel =
    !ghost || safeGhostProgress <= 0
      ? "--"
      : racerDelta > 0
        ? `You +${racerDelta}`
        : racerDelta < 0
          ? `Ghost +${Math.abs(racerDelta)}`
          : "Even";
  const stateLabel = isLive ? "Live race" : ghost ? "Personal best" : "No ghost yet";

  return (
    <Card className="race-card overflow-hidden rounded-lg border-white/10 backdrop-blur">
      <CardContent className="space-y-4 p-4">
        <div className="flex items-start justify-between gap-3">
          <div>
            <CardTitle className="flex items-center gap-2 font-mono text-xs uppercase tracking-[0.16em] text-cyan-100">
              <Trophy className="h-4 w-4" />
              Race Status
            </CardTitle>
            <div className="mt-2 ui-title text-2xl font-semibold text-foreground">{leadLabel}</div>
          </div>
          <Badge className="border-cyan-200/25 bg-cyan-200/10 text-cyan-100">{stateLabel}</Badge>
        </div>

        <div className="race-track" aria-label="Race progress">
          <div className="race-track-grid" />
          <div className="race-line race-line-you">
            <span className="race-fill race-fill-you" style={{ width: `${safeProgress}%` }} />
            <span className="race-runner race-runner-you" style={{ left: `${Math.min(Math.max(safeProgress, 5), 95)}%` }}>
              you
            </span>
          </div>
          <div className="race-line race-line-ghost">
            <span className="race-fill race-fill-ghost" style={{ width: `${safeGhostProgress}%` }} />
            <span className="race-runner race-runner-ghost" style={{ left: `${runnerPosition}%` }}>
              <Ghost className="h-3.5 w-3.5" />
            </span>
          </div>
          <Flag className="absolute right-3 top-1/2 h-4 w-4 -translate-y-1/2 text-rose-200/85" />
        </div>

        <div className="grid grid-cols-2 gap-2">
          <RaceMetric label="Your pace" value={stats.wpm} unit="wpm" tone="bright" />
          <RaceMetric label="Best pace" value={ghost?.stats.wpm ?? "--"} unit={ghost ? "wpm" : ""} />
          <RaceMetric label="Accuracy" value={`${stats.accuracy}%`} tone="bright" />
          <RaceMetric label="Chars" value={`${typedCount}/${targetCount}`} />
        </div>

        <div className="race-summary">
          <span>{timerSeconds}s timer</span>
          <strong>{ghost ? `PB ${formatTime(ghost.durationMs)} / ${ghost.stats.accuracy}%` : "Complete a run to set PB"}</strong>
        </div>
      </CardContent>
    </Card>
  );
}

function RaceMetric({
  label,
  value,
  unit,
  tone = "muted"
}: {
  label: string;
  value: ReactNode;
  unit?: string;
  tone?: "muted" | "bright";
}) {
  return (
    <div className="race-metric">
      <div className="font-mono text-[10px] uppercase tracking-[0.14em] text-muted-foreground">{label}</div>
      <div className={cn("ui-title mt-1 text-xl font-semibold", tone === "bright" ? "text-foreground" : "text-slate-300")}>
        {value}
        {unit ? <span className="ml-1 font-mono text-[10px] uppercase tracking-[0.12em] text-muted-foreground">{unit}</span> : null}
      </div>
    </div>
  );
}

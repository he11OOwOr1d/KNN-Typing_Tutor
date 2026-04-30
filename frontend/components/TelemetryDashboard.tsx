"use client";

import { useEffect, useState, useMemo } from "react";
import { Area, ComposedChart, Line, CartesianGrid, ResponsiveContainer, Tooltip, XAxis, YAxis, Legend } from "recharts";
import { Activity, AlertTriangle, Gauge, Keyboard } from "lucide-react";
import { Card, CardContent, CardTitle } from "@/components/ui/card";
import { cn } from "@/lib/utils";

const apiBase = process.env.NEXT_PUBLIC_API_URL || "http://localhost:4000";

type Attempt = {
  id: string;
  createdAt: string;
  durationMs: number;
  text?: string;
  typed?: string;
  timestamps?: number[];
  stats: {
    wpm: number;
    accuracy: number;
  };
};

type HeatmapData = Record<string, { total: number; errors: number }>;

const keyboardLayout = [
  ["q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]"],
  ["a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'"],
  ["z", "x", "c", "v", "b", "n", "m", ",", ".", "/"],
  [" "]
];

function HeatmapVisual({ data }: { data: HeatmapData }) {
  const maxErrors = Math.max(...Object.values(data).map(d => d.errors), 1);

  return (
    <div className="flex flex-col items-center gap-2 rounded-lg border border-white/10 bg-black/20 p-6 backdrop-blur">
      {keyboardLayout.map((row, rowIndex) => (
        <div key={rowIndex} className="flex gap-2">
          {row.map((key) => {
            const stats = data[key.toLowerCase()] || { total: 0, errors: 0 };
            const errorRate = stats.total > 0 ? stats.errors / stats.total : 0;
            const intensity = stats.errors > 0 ? Math.min(stats.errors / maxErrors, 1) : 0;
            const isSpace = key === " ";
            const bgColor = intensity > 0
              ? `rgba(239, 68, 68, ${0.2 + intensity * 0.6})`
              : "rgba(255, 255, 255, 0.05)";
            const borderColor = intensity > 0
              ? `rgba(239, 68, 68, ${0.4 + intensity * 0.4})`
              : "rgba(255, 255, 255, 0.1)";

            return (
              <div
                key={key}
                className={cn(
                  "relative flex items-center justify-center rounded-md border text-sm font-medium transition-colors hover:brightness-125",
                  isSpace ? "w-64 h-12" : "w-10 h-12 sm:w-12 sm:h-12"
                )}
                style={{
                  backgroundColor: bgColor,
                  borderColor: borderColor,
                  boxShadow: intensity > 0.5 ? `0 0 ${intensity * 15}px rgba(239, 68, 68, 0.4)` : "none"
                }}
                title={`Key: '${key}'\nErrors: ${stats.errors}\nTotal pressed: ${stats.total}\nError Rate: ${(errorRate * 100).toFixed(1)}%`}
              >
                {!isSpace && <span className={intensity > 0.3 ? "text-red-50" : "text-muted-foreground"}>{key}</span>}
                {stats.errors > 0 && !isSpace && (
                  <span className="absolute -top-2 -right-2 flex h-5 w-5 items-center justify-center rounded-full bg-red-500 text-[9px] font-bold text-white shadow-sm ring-2 ring-background">
                    {stats.errors}
                  </span>
                )}
              </div>
            );
          })}
        </div>
      ))}
      <div className="mt-4 flex items-center gap-3 text-xs text-muted-foreground">
        <span>Low Error Rate</span>
        <div className="h-2 w-32 rounded-full bg-gradient-to-r from-white/10 to-red-500/80" />
        <span>High Error Rate</span>
      </div>
    </div>
  );
}

export function TelemetryDashboard() {
  const [attempts, setAttempts] = useState<Attempt[]>([]);
  const [heatmap, setHeatmap] = useState<HeatmapData>({});
  const [isLoading, setIsLoading] = useState(true);

  useEffect(() => {
    async function loadData() {
      setIsLoading(true);
      try {
        const [attemptsRes, heatmapRes] = await Promise.all([
          fetch(`${apiBase}/api/attempts`),
          fetch(`${apiBase}/api/stats/heatmap`)
        ]);
        const attemptsData = await attemptsRes.json();
        const heatmapData = await heatmapRes.json();
        setAttempts(attemptsData.attempts || []);
        setHeatmap(heatmapData.heatmap || {});
      } catch (error) {
        console.error("Failed to load telemetry data:", error);
      } finally {
        setIsLoading(false);
      }
    }
    void loadData();
  }, []);

  // Sort chronologically and label by attempt number
  const sortedAttempts = useMemo(
    () => [...attempts].sort((a, b) => new Date(a.createdAt).getTime() - new Date(b.createdAt).getTime()),
    [attempts]
  );

  // Per-second WPM breakdown for the last 5 attempts, overlaid on the same chart
  const RUN_COLORS = ["#06b6d4", "#a78bfa", "#34d399", "#fb923c", "#f472b6"];
  const recentAttempts = sortedAttempts.slice(-5);

  const perSecondData = useMemo(() => {
    if (recentAttempts.length === 0) return [];
    const maxDuration = Math.max(...recentAttempts.map(a => Math.floor(a.durationMs / 1000)));
    const data: Record<string, any>[] = [];

    for (let s = 4; s <= maxDuration; s += 4) {
      const row: Record<string, any> = { name: `${s}s` };
      recentAttempts.forEach((attempt, i) => {
        if (!attempt.text || !attempt.timestamps) return;
        const maxSec = Math.floor(attempt.durationMs / 1000);
        if (s > maxSec) return;
        const timeLimit = s * 1000;
        let keysBefore = attempt.timestamps.findIndex(t => t > timeLimit);
        if (keysBefore === -1) keysBefore = attempt.timestamps.length;
        const typedUpTo = (attempt.typed || "").slice(0, keysBefore);
        const targetUpTo = attempt.text.slice(0, keysBefore);
        let correct = 0;
        for (let j = 0; j < typedUpTo.length; j++) {
          if (typedUpTo[j] === targetUpTo[j]) correct++;
        }
        row[`run${i}`] = Math.round(correct / 5 / (s / 60));
      });
      data.push(row);
    }
    return data;
  }, [recentAttempts]);

  // Fatigue curve from the latest attempt's per-second WPM
  const latestAttempt = sortedAttempts[sortedAttempts.length - 1] ?? null;

  const fatigueData = useMemo(() => {
    if (!latestAttempt?.text || !latestAttempt.timestamps) return [];

    const target = latestAttempt.text;
    const typedStr = latestAttempt.typed || "";
    const stamps = latestAttempt.timestamps;
    const maxSecs = Math.floor(latestAttempt.durationMs / 1000);

    const history: { time: number; wpm: number; trend?: number; name: string }[] = [];

    for (let s = 1; s <= maxSecs; s++) {
      const timeLimit = s * 1000;
      let keysBefore = stamps.findIndex((t) => t > timeLimit);
      if (keysBefore === -1) keysBefore = stamps.length;

      const typedUpTo = typedStr.slice(0, keysBefore);
      const targetUpTo = target.slice(0, keysBefore);
      let correct = 0;
      for (let i = 0; i < typedUpTo.length; i++) {
        if (typedUpTo[i] === targetUpTo[i]) correct++;
      }
      const wpm = Math.round(correct / 5 / (s / 60));
      history.push({ time: s, wpm, name: `${s}s` });
    }

    if (history.length >= 5) {
      let sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;
      const n = history.length;
      for (const p of history) { sumX += p.time; sumY += p.wpm; sumXY += p.time * p.wpm; sumX2 += p.time * p.time; }
      const xMean = sumX / n;
      const yMean = sumY / n;
      const denom = sumX2 - n * xMean * xMean;
      if (denom !== 0) {
        const slope = (sumXY - n * xMean * yMean) / denom;
        const intercept = yMean - slope * xMean;
        for (const p of history) { p.trend = Math.round(slope * p.time + intercept); }
      }
    }

    return history;
  }, [latestAttempt]);

  const avgWpm = attempts.length ? Math.round(attempts.reduce((a, c) => a + c.stats.wpm, 0) / attempts.length) : 0;
  const avgAccuracy = attempts.length ? Math.round(attempts.reduce((a, c) => a + c.stats.accuracy, 0) / attempts.length) : 0;
  const totalErrors = Object.values(heatmap).reduce((a, c) => a + c.errors, 0);

  if (isLoading) {
    return (
      <div className="flex h-64 items-center justify-center">
        <div className="h-8 w-8 animate-spin rounded-full border-4 border-primary border-t-transparent" />
      </div>
    );
  }

  if (attempts.length === 0) {
    return (
      <div className="flex h-64 flex-col items-center justify-center gap-3 text-muted-foreground">
        <Activity className="h-10 w-10 opacity-30" />
        <p className="text-sm">No attempts recorded yet. Complete a race to see your analytics!</p>
      </div>
    );
  }

  return (
    <div className="space-y-6 animate-in fade-in slide-in-from-bottom-4 duration-500">
      {/* Summary Stats */}
      <div className="grid gap-4 md:grid-cols-3">
        <Card className="border-white/10 bg-black/20 backdrop-blur">
          <CardContent className="p-6">
            <div className="flex items-center gap-2 text-sm font-medium text-muted-foreground">
              <Gauge className="h-4 w-4" /> Average WPM
            </div>
            <div className="mt-2 text-3xl font-bold">{avgWpm}</div>
          </CardContent>
        </Card>
        <Card className="border-white/10 bg-black/20 backdrop-blur">
          <CardContent className="p-6">
            <div className="flex items-center gap-2 text-sm font-medium text-muted-foreground">
              <Activity className="h-4 w-4" /> Average Accuracy
            </div>
            <div className="mt-2 text-3xl font-bold">{avgAccuracy}%</div>
          </CardContent>
        </Card>
        <Card className="border-white/10 bg-black/20 backdrop-blur">
          <CardContent className="p-6">
            <div className="flex items-center gap-2 text-sm font-medium text-muted-foreground">
              <AlertTriangle className="h-4 w-4 text-red-400" /> Total Errors
            </div>
            <div className="mt-2 text-3xl font-bold">{totalErrors}</div>
          </CardContent>
        </Card>
      </div>

      {/* Fatigue Curve (latest run) */}
      {fatigueData.length > 0 && (
        <Card className="border-white/10 bg-black/20 backdrop-blur">
          <CardContent className="p-6">
            <CardTitle className="mb-1 flex items-center gap-2 text-lg">
              <Activity className="h-5 w-5 text-amber-400" /> Latest Run — Fatigue Curve
            </CardTitle>
            <p className="mb-6 text-xs text-muted-foreground">Per-second WPM with linear regression trendline. A falling red line = fatigue.</p>
            <div className="h-[280px] w-full">
              <ResponsiveContainer width="100%" height="100%">
                <ComposedChart data={fatigueData} margin={{ top: 10, right: 20, left: 10, bottom: 15 }}>
                  <defs>
                    <linearGradient id="fatigueGrad" x1="0" y1="0" x2="0" y2="1">
                      <stop offset="5%" stopColor="#f59e0b" stopOpacity={0.3} />
                      <stop offset="95%" stopColor="#f59e0b" stopOpacity={0} />
                    </linearGradient>
                  </defs>
                  <CartesianGrid strokeDasharray="3 3" stroke="rgba(255,255,255,0.08)" vertical={false} />
                  <XAxis dataKey="name" stroke="rgba(255,255,255,0.3)" fontSize={11} tickLine={false} axisLine={false} label={{ value: "Time (seconds)", position: "insideBottom", offset: -10, fill: "rgba(255,255,255,0.4)", fontSize: 11 }} />
                  <YAxis stroke="rgba(255,255,255,0.3)" fontSize={11} tickLine={false} axisLine={false} label={{ value: "WPM", angle: -90, position: "insideLeft", fill: "rgba(255,255,255,0.4)", fontSize: 11, style: { textAnchor: "middle" } }} />
                  <Tooltip contentStyle={{ backgroundColor: "#0f172a", borderColor: "rgba(255,255,255,0.1)", borderRadius: "8px" }} itemStyle={{ color: "#f8fafc", fontSize: 12 }} />
                  <Legend verticalAlign="top" height={36} wrapperStyle={{ fontSize: "12px", opacity: 0.75 }} />
                  <Area type="monotone" dataKey="wpm" name="WPM / second" stroke="#f59e0b" strokeWidth={2} fill="url(#fatigueGrad)" fillOpacity={1} dot={false} />
                  <Line type="linear" dataKey="trend" name="Fatigue Trendline" stroke="#ef4444" strokeWidth={2} strokeDasharray="5 5" dot={false} activeDot={false} />
                </ComposedChart>
              </ResponsiveContainer>
            </div>
          </CardContent>
        </Card>
      )}

      {/* Per-second WPM timeline — last 5 runs */}
      {perSecondData.length > 0 && (
        <Card className="border-white/10 bg-black/20 backdrop-blur">
          <CardContent className="p-6">
            <CardTitle className="mb-1 flex items-center gap-2 text-lg">
              <Gauge className="h-5 w-5 text-cyan-400" /> WPM Timeline — Recent Runs
            </CardTitle>
            <p className="mb-6 text-xs text-muted-foreground">Per-second WPM across your last {recentAttempts.length} run{recentAttempts.length !== 1 ? "s" : ""}. Compare how your speed changes throughout each session.</p>
            <div className="h-[300px] w-full">
              <ResponsiveContainer width="100%" height="100%">
                <ComposedChart data={perSecondData} margin={{ top: 10, right: 20, left: 10, bottom: 15 }}>
                  <CartesianGrid strokeDasharray="3 3" stroke="rgba(255,255,255,0.08)" vertical={false} />
                  <XAxis dataKey="name" stroke="rgba(255,255,255,0.3)" fontSize={11} tickLine={false} axisLine={false} label={{ value: "Time (seconds)", position: "insideBottom", offset: -10, fill: "rgba(255,255,255,0.4)", fontSize: 11 }} />
                  <YAxis stroke="rgba(255,255,255,0.3)" fontSize={11} tickLine={false} axisLine={false} label={{ value: "WPM", angle: -90, position: "insideLeft", fill: "rgba(255,255,255,0.4)", fontSize: 11, style: { textAnchor: "middle" } }} />
                  <Tooltip contentStyle={{ backgroundColor: "#0f172a", borderColor: "rgba(255,255,255,0.1)", borderRadius: "8px" }} itemStyle={{ color: "#f8fafc", fontSize: 12 }} />
                  <Legend verticalAlign="top" height={36} wrapperStyle={{ fontSize: "12px", opacity: 0.75 }} />
                  {recentAttempts.map((attempt, i) => (
                    <Line
                      key={attempt.id}
                      type="monotone"
                      dataKey={`run${i}`}
                      name={`Run ${sortedAttempts.length - recentAttempts.length + i + 1} (${attempt.stats.wpm} wpm)`}
                      stroke={RUN_COLORS[i]}
                      strokeWidth={i === recentAttempts.length - 1 ? 2.5 : 1.5}
                      strokeOpacity={i === recentAttempts.length - 1 ? 1 : 0.5}
                      dot={false}
                      activeDot={{ r: 4 }}
                    />
                  ))}
                </ComposedChart>
              </ResponsiveContainer>
            </div>
          </CardContent>
        </Card>
      )}

      {/* Error Heatmap */}
      <Card className="border-white/10 bg-black/20 backdrop-blur">
        <CardContent className="p-6">
          <CardTitle className="mb-6 flex items-center gap-2 text-lg">
            <Keyboard className="h-5 w-5 text-primary" /> Error Heatmap
          </CardTitle>
          <HeatmapVisual data={heatmap} />
        </CardContent>
      </Card>
    </div>
  );
}

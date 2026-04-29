"use client";

import { useEffect, useState } from "react";
import { Area, ComposedChart, Line, CartesianGrid, ResponsiveContainer, Tooltip, XAxis, YAxis, Legend } from "recharts";
import { Activity, AlertTriangle, Clock, Gauge, Keyboard } from "lucide-react";
import { Card, CardContent, CardTitle } from "@/components/ui/card";
import { Badge } from "@/components/ui/badge";
import { cn } from "@/lib/utils";

const apiBase = process.env.NEXT_PUBLIC_API_URL || "http://localhost:4000";

type Attempt = {
  id: string;
  createdAt: string;
  durationMs: number;
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
            
            // Calculate color based on error intensity. Default is standard key color, higher intensity = more red/orange
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

const CustomTooltip = ({ active, payload, label }: any) => {
  if (active && payload && payload.length) {
    const data = payload[0].payload;
    return (
      <div className="rounded-lg border border-white/10 bg-[#0f172a] p-3 shadow-lg backdrop-blur">
        <p className="mb-2 font-semibold text-white">{label}</p>
        <div className="flex flex-col gap-1 text-sm">
          <p style={{ color: "#06b6d4" }}>WPM: <span className="font-mono font-medium">{data.wpm}</span></p>
          <p style={{ color: "#ef4444" }}>Error Rate: <span className="font-mono font-medium">{data.errors}%</span></p>
          <p className="text-muted-foreground">Duration: <span className="font-mono font-medium">{data.duration}s</span></p>
        </div>
      </div>
    );
  }
  return null;
};

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

  const sortedAttempts = [...attempts].sort((a, b) => a.durationMs - b.durationMs);

  const chartData = sortedAttempts.map((attempt) => {
    const durationSec = Math.round(attempt.durationMs / 1000);
    return {
      name: `${durationSec}s`,
      wpm: attempt.stats.wpm,
      accuracy: attempt.stats.accuracy,
      errors: Math.round((100 - attempt.stats.accuracy) * 10) / 10,
      duration: durationSec
    };
  });

  const avgWpm = attempts.length ? Math.round(attempts.reduce((acc, curr) => acc + curr.stats.wpm, 0) / attempts.length) : 0;
  const avgAccuracy = attempts.length ? Math.round(attempts.reduce((acc, curr) => acc + curr.stats.accuracy, 0) / attempts.length) : 0;
  const totalErrors = Object.values(heatmap).reduce((acc, curr) => acc + curr.errors, 0);

  if (isLoading) {
    return (
      <div className="flex h-64 items-center justify-center">
        <div className="h-8 w-8 animate-spin rounded-full border-4 border-primary border-t-transparent" />
      </div>
    );
  }

  return (
    <div className="space-y-6 animate-in fade-in slide-in-from-bottom-4 duration-500">
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

      <Card className="border-white/10 bg-black/20 backdrop-blur">
        <CardContent className="p-6">
          <CardTitle className="mb-6 flex items-center gap-2 text-lg">
            <Keyboard className="h-5 w-5 text-primary" /> Error Heatmap
          </CardTitle>
          <HeatmapVisual data={heatmap} />
        </CardContent>
      </Card>

      <Card className="border-white/10 bg-black/20 backdrop-blur">
        <CardContent className="p-6">
          <CardTitle className="mb-6 flex items-center gap-2 text-lg">
            <Activity className="h-5 w-5 text-primary" /> Performance by Test Duration
          </CardTitle>
          <div className="h-[300px] w-full">
            <ResponsiveContainer width="100%" height="100%">
              <ComposedChart data={chartData} margin={{ top: 10, right: 20, left: 20, bottom: 15 }}>
                <defs>
                  <linearGradient id="colorWpm" x1="0" y1="0" x2="0" y2="1">
                    <stop offset="5%" stopColor="#06b6d4" stopOpacity={0.3}/>
                    <stop offset="95%" stopColor="#06b6d4" stopOpacity={0}/>
                  </linearGradient>
                </defs>
                <CartesianGrid strokeDasharray="3 3" stroke="rgba(255,255,255,0.1)" vertical={false} />
                <XAxis 
                  dataKey="name" 
                  stroke="rgba(255,255,255,0.4)" 
                  fontSize={12}
                  tickLine={false}
                  axisLine={false}
                  dy={5}
                  label={{ value: 'Test Duration (Seconds)', position: 'insideBottom', offset: -10, fill: 'rgba(255,255,255,0.6)' }}
                />
                <YAxis 
                  yAxisId="left" 
                  stroke="rgba(255,255,255,0.4)" 
                  fontSize={12}
                  tickLine={false}
                  axisLine={false}
                  label={{ value: 'WPM', angle: -90, position: 'insideLeft', fill: 'rgba(255,255,255,0.6)', style: { textAnchor: 'middle' } }}
                />
                <YAxis 
                  yAxisId="right" 
                  orientation="right" 
                  stroke="rgba(255,255,255,0.4)" 
                  fontSize={12}
                  tickLine={false}
                  axisLine={false}
                  label={{ value: 'Error Rate (%)', angle: 90, position: 'insideRight', fill: 'rgba(255,255,255,0.6)', style: { textAnchor: 'middle' } }}
                />
                <Tooltip content={<CustomTooltip />} />
                <Legend verticalAlign="top" height={36} wrapperStyle={{ fontSize: "13px", opacity: 0.8 }} />
                <Area 
                  yAxisId="left"
                  type="monotone" 
                  dataKey="wpm" 
                  name="WPM"
                  stroke="#06b6d4" 
                  strokeWidth={2}
                  fillOpacity={1} 
                  fill="url(#colorWpm)" 
                />
                <Line 
                  yAxisId="right"
                  type="monotone" 
                  dataKey="errors" 
                  name="Error %"
                  stroke="#ef4444" 
                  strokeWidth={2}
                  strokeDasharray="5 5"
                  dot={{ r: 3, fill: "#ef4444" }}
                  activeDot={{ r: 5 }}
                />
              </ComposedChart>
            </ResponsiveContainer>
          </div>
        </CardContent>
      </Card>
    </div>
  );
}

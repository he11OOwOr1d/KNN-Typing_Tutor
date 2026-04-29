import express from "express";
import cors from "cors";
import { promises as fs } from "node:fs";
import { randomUUID } from "node:crypto";
import path from "node:path";
import { fileURLToPath } from "node:url";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const dataFile = path.join(__dirname, "../data/attempts.json");

const app = express();
const port = process.env.PORT || 4000;
const host = process.env.HOST || "0.0.0.0";

const allowedOrigins = (process.env.FRONTEND_ORIGIN || "http://localhost:3000").split(",");
app.use(cors({ origin: allowedOrigins }));
app.use(express.json({ limit: "1mb" }));

const sentences = [
  {
    id: "calm-systems",
    text: "Calm systems reward steady focus, clean feedback, and small decisions made with care."
  },
  {
    id: "future-interface",
    text: "The future interface disappears into rhythm, letting your hands think before your eyes catch up."
  },
  {
    id: "practice-loop",
    text: "Practice becomes addictive when progress feels visible, precise, and honest in the moment."
  },
  {
    id: "quiet-speed",
    text: "Quiet speed is built from patient repetitions, fewer corrections, and trust in the next key."
  }
];

async function readAttempts() {
  try {
    const raw = await fs.readFile(dataFile, "utf8");
    return JSON.parse(raw);
  } catch (error) {
    if (error.code === "ENOENT") {
      return [];
    }

    throw error;
  }
}

async function writeAttempts(attempts) {
  await fs.mkdir(path.dirname(dataFile), { recursive: true });
  await fs.writeFile(dataFile, JSON.stringify(attempts, null, 2));
}

function calculateStats(text, typed, durationMs) {
  const chars = [...typed];
  const target = [...text];
  const correct = chars.filter((char, index) => char === target[index]).length;
  const accuracy = chars.length ? Math.round((correct / chars.length) * 100) : 100;
  const minutes = Math.max(durationMs / 60000, 1 / 60);
  const wpm = Math.round(correct / 5 / minutes);

  return { correct, accuracy, wpm };
}

app.get("/api/health", (_request, response) => {
  response.json({ ok: true });
});

app.get("/api/sentences", (_request, response) => {
  response.json({ sentences });
});

app.get("/api/sentences/:id/ghost", async (request, response, next) => {
  try {
    const attempts = await readAttempts();
    const best = attempts
      .filter((attempt) => attempt.sentenceId === request.params.id && attempt.completed)
      .sort((a, b) => {
        const wpmDelta = (b.stats?.wpm || 0) - (a.stats?.wpm || 0);
        if (wpmDelta !== 0) {
          return wpmDelta;
        }

        const accuracyDelta = (b.stats?.accuracy || 0) - (a.stats?.accuracy || 0);
        if (accuracyDelta !== 0) {
          return accuracyDelta;
        }

        const durationDelta = (a.durationMs || Number.POSITIVE_INFINITY) - (b.durationMs || Number.POSITIVE_INFINITY);
        if (durationDelta !== 0) {
          return durationDelta;
        }

        return new Date(b.createdAt).getTime() - new Date(a.createdAt).getTime();
      })[0];

    response.json({ ghost: best || null });
  } catch (error) {
    next(error);
  }
});

app.get("/api/attempts", async (_request, response, next) => {
  try {
    const attempts = await readAttempts();
    response.json({ attempts: attempts.slice(-100) });
  } catch (error) {
    next(error);
  }
});

app.get("/api/stats/heatmap", async (_request, response, next) => {
  try {
    const attempts = await readAttempts();
    const heatmap = {};

    for (const attempt of attempts) {
      if (Array.isArray(attempt.keystrokes)) {
        for (const ks of attempt.keystrokes) {
          if (!heatmap[ks.key]) {
            heatmap[ks.key] = { total: 0, errors: 0 };
          }
          heatmap[ks.key].total += 1;
          if (ks.isError) {
            heatmap[ks.key].errors += 1;
          }
        }
      }
    }

    response.json({ heatmap });
  } catch (error) {
    next(error);
  }
});

app.post("/api/attempts", async (request, response, next) => {
  try {
    const { sentenceId, typed, timestamps, durationMs, keystrokes = [] } = request.body;
    const sentence = sentences.find((item) => item.id === sentenceId);

    if (!sentence) {
      return response.status(404).json({ error: "Sentence not found" });
    }

    if (typeof typed !== "string" || !Array.isArray(timestamps) || typeof durationMs !== "number" || !Array.isArray(keystrokes)) {
      return response.status(400).json({ error: "Invalid attempt payload" });
    }

    const safeTimestamps = timestamps.map((value) => Number(value)).filter(Number.isFinite);
    const stats = calculateStats(sentence.text, typed, durationMs);
    const attempts = await readAttempts();
    const attempt = {
      id: randomUUID(),
      sentenceId,
      text: sentence.text,
      typed,
      timestamps: safeTimestamps,
      durationMs,
      keystrokes,
      completed: typed.length === sentence.text.length,
      createdAt: new Date().toISOString(),
      stats
    };

    attempts.push(attempt);
    await writeAttempts(attempts.slice(-250));

    response.status(201).json({ attempt });
  } catch (error) {
    next(error);
  }
});

app.use((error, _request, response, _next) => {
  console.error(error);
  response.status(500).json({ error: "Something went wrong" });
});

app.listen(port, host, () => {
  console.log(`Typing tutor backend running on http://${host}:${port}`);
});

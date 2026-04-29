import { cn } from "@/lib/utils";

export function Spotlight({ className }: { className?: string }) {
  return (
    <div
      aria-hidden="true"
      className={cn(
        "pointer-events-none absolute inset-x-0 top-0 h-[420px] overflow-hidden",
        "bg-[linear-gradient(110deg,rgba(59,130,246,0.22),transparent_34%,rgba(168,85,247,0.12)_66%,transparent)]",
        "after:absolute after:inset-0 after:bg-[linear-gradient(to_bottom,transparent,rgba(5,7,13,0.62),hsl(var(--background)))]",
        className
      )}
    />
  );
}

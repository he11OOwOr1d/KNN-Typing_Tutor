import * as React from "react";
import { cn } from "@/lib/utils";

export function Badge({ className, ...props }: React.HTMLAttributes<HTMLDivElement>) {
  return (
    <div
      className={cn(
        "inline-flex items-center rounded-sm border border-border bg-muted/70 px-2.5 py-1 font-mono text-[11px] font-medium uppercase tracking-[0.18em] text-muted-foreground",
        className
      )}
      {...props}
    />
  );
}

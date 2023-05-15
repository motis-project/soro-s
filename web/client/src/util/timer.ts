export class timer {
  constructor() {
    this.start = Date.now();
  }

  elapsed(): number {
    return Date.now() - this.start;
  }

  printElapsed(message: string): void {
    // eslint-disable-next-line no-console
    console.log('[' + message + ']:', this.elapsed());
  }

  start: number;
}

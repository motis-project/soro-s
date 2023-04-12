export function devMode(): boolean {
    // @ts-ignore
    return import.meta.env.DEV;
}
export function getIsDev() : boolean {
    return process.env.NODE_ENV === "development"
}
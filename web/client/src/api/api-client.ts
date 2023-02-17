// @ts-ignore
const runningInDevMode = import.meta.env.DEV;

export const transformUrl = (relativeUrl: string) => {
    const noSlashUrl = relativeUrl.startsWith('/') ? relativeUrl.slice(1) : relativeUrl;
    if (!runningInDevMode) {
        return `${window.origin}/${noSlashUrl}`;
    }
  
    return `${window.origin}/api/${noSlashUrl}`;
};

export const sendRequest = ({ url, options }: { url: string, options?: RequestInit }) =>
    fetch(transformUrl(url), options);

export const sendPostData = ({ url, data }: { url: string, data: object }) =>
    sendRequest({
        url,
        options: {
            method: 'POST',
            body: JSON.stringify(data),
            headers: { 'Content-Type': 'application/json' },
        },
    });

export const transformUrl = (relativeUrl: string) => {
    const noSlashUrl = relativeUrl.startsWith('/') ? relativeUrl.slice(1) : relativeUrl;
    // @ts-ignore
    if (!import.meta.env.DEV) {
        return `${window.origin}/${noSlashUrl}`;
    }
  
    return `${window.origin}/api/${noSlashUrl}`;
};

export const sendRequest = async ({ url, options }: { url: string, options?: RequestInit }) =>
    fetch(transformUrl(url), options);

export const sendPostData = async ({ url, data }: { url: string, data: object }) =>
    sendRequest({
        url,
        options: {
            method: 'POST',
            body: JSON.stringify(data),
            headers: { 'Content-Type': 'application/json' },
        },
    });

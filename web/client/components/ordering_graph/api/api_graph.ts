export const transformUrl = (url: string, values?: Array<string> ) => {
    if(values !== undefined) {
        return `${window.origin}${url}?from=${values[0]}&to=${values[1]}`;
    }
    return `${window.origin}${url}`;
}

export const sendRequest = ({ url, options, values }: { url: string, options?: RequestInit, values?: Array<string> }) =>
    fetch(transformUrl(url, values), options);

export const sendPostData = ({ url, data, values }: { url: string, data: object, values: Array<string> }) =>
    sendRequest({
        url,
        options: {
            method: 'POST',
            body: JSON.stringify(data),
            headers: { 'Content-Type': 'application/json' },
        },
        values
    });

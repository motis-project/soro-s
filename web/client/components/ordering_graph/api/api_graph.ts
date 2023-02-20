export const getGeneratedGraph = ({ url }: { url: string }) =>
    fetch(url).then(response => response.json());

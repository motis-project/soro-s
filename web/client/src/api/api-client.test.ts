import { transformUrl, sendRequest, sendPostData } from './api-client';

describe('api-client', async () => {
    beforeEach(async () => {
        window.fetch = vi.fn(async () => new Response());
        vi.clearAllMocks();
    });

    describe('transformUrl()', async () => {
        it('returns the given url appended to the origin when not running in vite DEV mode', async () => {
            // @ts-ignore
            import.meta.env.DEV = '';

            expect(transformUrl('/some/slash-url')).toBe(`${window.origin}/some/slash-url`);
            expect(transformUrl('some-non-slash-url')).toBe(`${window.origin}/some-non-slash-url`);
        });

        it('returns the given url appended to the origin with an \'api\' infix when running in vite DEV mode', async () => {
            // @ts-ignore
            import.meta.env.DEV = true;

            expect(transformUrl('/some/slash-url')).toBe(`${window.origin}/api/some/slash-url`);
            expect(transformUrl('some-non-slash-url')).toBe(`${window.origin}/api/some-non-slash-url`);
        });
    });

    it('uses the \'fetch\' api and a url transform when sending a request', async () => {
        await sendRequest({
            url: 'some-url',
            options: { body: 'some-body' },
        });

        expect(window.fetch).toHaveBeenCalledWith(
            `${window.origin}/api/some-url`,
            { body: 'some-body' },
        );
    });

    it('uses the \'fetch\' api and a url transform when posting data', async () => {
        await sendPostData({
            url: 'some-url',
            data: { some: { wonderful: 'data' } },
        });

        expect(window.fetch).toHaveBeenCalledWith(
            `${window.origin}/api/some-url`,
            {
                method: 'POST',
                body: JSON.stringify({ some: { wonderful: 'data' } }),
                headers: { 'Content-Type': 'application/json' },
            },
        );
    });
});

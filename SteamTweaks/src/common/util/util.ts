export const fetchWithTimeout = async (input: RequestInfo | URL, init?: RequestInit & { timeout: number }) =>  {
    const { timeout = 8000 } = init || {};
    
    const controller = new AbortController();
    const id = setTimeout(() => controller.abort(), timeout);
    const response = await fetch(input, {
      ...(init ||{}),
      signal: controller.signal  
    });
    clearTimeout(id);
    return response;
  }
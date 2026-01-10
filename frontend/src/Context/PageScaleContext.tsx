
import { createContext, useContext, useMemo, useReducer } from 'react';

interface PageScaleContextState {
  TILE_SIZE: number;
  BUFFER_TILES: number;
  MIN_SCALE: number;
  MAX_SCALE: number;
}

type PageScaleAction = 
    | { type: 'INCREMENT'; payload: {property: keyof PageScaleContextState; value: number}}
    | { type: 'DECREMENT'; payload: {property: keyof PageScaleContextState; value: number}}
    | { type: 'RESET'};

interface PageScaleDispatch {
    pageScaleContext: PageScaleContextState;
    dispatchPageScaleContext: React.Dispatch<PageScaleAction>;
}

const initialPageScaleState : PageScaleContextState =  {
  TILE_SIZE: 200,
  BUFFER_TILES: 2,
  MIN_SCALE: 0.2,
  MAX_SCALE: 4,
}

export const PageScaleContext = createContext<PageScaleDispatch | null>(null);

export default function PageScaleContextProvider({children}: {children: React.ReactNode}) {
    function pageScaleReducer(state: PageScaleContextState, action: PageScaleAction) : PageScaleContextState {
        switch(action.type) {
            case 'INCREMENT':
                return { ...state, [action.payload.property]: state[action.payload.property] + action.payload.value };
            case 'DECREMENT':
                return { ...state, [action.payload.property]: state[action.payload.property] - action.payload.value };
            case 'RESET':
                return initialPageScaleState;
            default:
                return state;
        }
    }
    const [pageScaleContext, dispatchPageScaleContext] = useReducer(pageScaleReducer, initialPageScaleState)

    const settings = useMemo(() => ({ pageScaleContext, dispatchPageScaleContext }), [pageScaleContext]);


    return (<PageScaleContext.Provider value={settings}>
        {children}
    </PageScaleContext.Provider>);
}

export function usePageScaleContext() {
    const context = useContext(PageScaleContext)
    if (context == null) {
        throw new Error('usePageScaleContext must be used within a PageScaleContextProvider');
    }

    return context
}
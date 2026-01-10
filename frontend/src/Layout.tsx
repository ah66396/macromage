import PageScaleContextProvider from "./Context/PageScaleContext"
import App from "./App"

export default function Layout() {
    return <PageScaleContextProvider>
        <App/>
    </PageScaleContextProvider>
}
## SORO-S x GoldenLayoutV2

SORO-S uses [GoldenLayout](https://github.com/golden-layout/golden-layout) to provide window management in the browser.

### Add a new component
To add a new component type to golden layout, follow these steps:
1. Provide a new Vue component in [components](components). You should be able to just copy and paste an existing
    component. Inside this component, you can build your Vue DOM as usual.
2. Register a technical name for the new GL component in [the golden layout constants file](golden-layout-constants.ts)
3. Provide the component name and a default title as enforced in the two constants

### Usage

To add a tab with a given component to the application, import the action `addGoldenLayoutTab` from the [golden layout store](src/stores/golden-layout-store.ts):

```js
import { GoldenLayoutNamespace } from "@/stores/golden-layout-store.ts";
import { mapActions } from "vuex";

export default {
    template: '<button @click="addTab"></button>',
    actions: {
        addTab() {
            this.addGoldenLayoutTab({
                // Example, put your technical name here
                componentTechnicalName: ComponentTechnicalName.INFRASTRUCTURE,
                // Can also be omitted, in that case the default title will be applied
                title: 'My new Infrastructure Tab'
            })
        },

        ...mapActions(GoldenLayoutNamespace, ['addGoldenLayoutTab']),
    }
}
```


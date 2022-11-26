<template>
  <div>
    <h2>This is tt.</h2>
  </div>
</template>

<script>
export default {
  name: "TimetableComponent",

  data() {
    return {
      container: undefined,
      componentState: undefined,
    }
  },

  methods: {
    setContainer(container) {
      this.container = container;
    },

    setComponentState(componentState) {
      this.componentState = componentState;
    },
  }
}
</script>

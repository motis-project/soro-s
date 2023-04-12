<template>
  <div>
    <extended-search class="search-field" :show-extended-options="true" />
  </div>
</template>

<script setup lang="ts">
import ExtendedSearch from './extended-search.vue';
</script>

<script lang="ts">
import { defineComponent } from 'vue';

export default defineComponent({
  name: 'SidebarSearch'
});
</script>

<style>
.search-field {
  padding: 3%;
}
</style>

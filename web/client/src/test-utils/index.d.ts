import { Module } from 'vuex';

export type Configuration = {
    props?: any,
    data?: any,
    mixins?: any,
    global?: any,
    mocks?: any,
    filters?: any,
    injections?: any,
    stubs?: any,
    store?: StoreConfiguration,
};

export type StoreConfiguration = Record<string, Module<any, any>>;
